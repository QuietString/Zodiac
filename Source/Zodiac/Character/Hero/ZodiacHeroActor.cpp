// the.quiet.string@gmail.com


#include "ZodiacHeroActor.h"

#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Hero/ZodiacHeroAbilitySystemComponent.h"
#include "AbilitySystem/Host/ZodiacHostAbilitySystemComponent.h"
#include "Animation/ZodiacHeroAnimInstance.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "Character/ZodiacHealthComponent.h"
#include "AbilitySystem/Hero/ZodiacHeroAbilityManagerComponent.h"
#include "Character/Hero/ZodiacHeroData.h"
#include "ZodiacHeroSkeletalMeshComponent.h"
#include "ZodiacLogChannels.h"
#include "Character/Host/ZodiacHostCharacter.h"
#include "Character/Host/ZodiacHostExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Net/UnrealNetwork.h"
#include "Player/ZodiacPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroActor)

using namespace ZodiacGameplayTags;

const FName AZodiacHeroActor::NAME_ActorFeatureName("Hero");
const TArray<FGameplayTag> AZodiacHeroActor::StateChain = { InitState_Spawned, InitState_DataAvailable, InitState_DataInitialized,InitState_GameplayReady };

AZodiacHeroActor::AZodiacHeroActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;	

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;
	
	Mesh = CreateDefaultSubobject<UZodiacHeroSkeletalMeshComponent>(TEXT("Mesh"));
	if (Mesh)
	{
		Mesh->SetupAttachment(RootComponent);
		Mesh->bIsHeroHidden = true;
		Mesh->ClothTeleportMode = EClothingTeleportMode::TeleportAndReset;
		Mesh->AlwaysLoadOnClient = true;
		Mesh->AlwaysLoadOnServer = true;
		Mesh->bOwnerNoSee = false;
		Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
		Mesh->bCastDynamicShadow = true;
		Mesh->bAffectDynamicIndirectLighting = true;
		Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		Mesh->SetGenerateOverlapEvents(false);
		Mesh->SetCanEverAffectNavigation(false);
	}
	
	AbilitySystemComponent = CreateDefaultSubobject<UZodiacHeroAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AbilityManagerComponent = CreateDefaultSubobject<UZodiacHeroAbilityManagerComponent>(TEXT("AbilityManagerComponent"));

	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHealthComponent>(this, TEXT("HealthComponent"));
}

bool AZodiacHeroActor::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	// @TODO: IDK why but without logs, suddenly InitState doesn't work. 
	check(Manager);

	if (!CurrentState.IsValid() && DesiredState == InitState_Spawned)
	{
		UE_LOG_WITH_ROLE(LogZodiacFramework, Warning, TEXT("Hero actor success to change state to %s"), *DesiredState.ToString());
		return true;
	}
	else if (CurrentState == InitState_Spawned && DesiredState == InitState_DataAvailable)
	{
		// Hero data is required.
		if (!HeroData)
		{
			UE_LOG_WITH_ROLE(LogZodiacFramework, Log, TEXT("Hero Actor failed to change state to %s. Reason: no hero data"), *DesiredState.ToString());
			return false;
		}
		
		// Host Character is required.
		AZodiacHostCharacter* Host = HostCharacter.Get();
		if (Host)
		{
			// Host ASC is required.
			if (!Host->GetHostAbilitySystemComponent())
			{
				UE_LOG_WITH_ROLE(LogZodiacFramework, Log, TEXT("Hero actor failed to change state to %s. Reason: no host asc"), *DesiredState.ToString());
				return false;
			}
		}
		else
		{
			UE_LOG_WITH_ROLE(LogZodiacFramework, Log, TEXT("Hero actor failed to change state to %s. Reason: no host"), *DesiredState.ToString());
			return false;
		}

		const bool bHasAuthority = Host->HasAuthority();
		const bool bIsLocallyControlled = Host->IsLocallyControlled();

		if (bHasAuthority || bIsLocallyControlled)
		{
			// Check for being possessed by a controller.
			if (!Host->GetController<AController>())
			{
				UE_LOG_WITH_ROLE(LogZodiacFramework, Log, TEXT("Hero actor failed to change state to %s. Reason: no controller"), *DesiredState.ToString());
				return false;
			}
		}
		
		UE_LOG_WITH_ROLE(LogZodiacFramework, Warning, TEXT("Hero actor success to change state to %s"), *CurrentState.ToString());
		return true;
	}
	else if (CurrentState == InitState_DataAvailable && DesiredState == InitState_DataInitialized)
	{
		// Transition to initialize if all features have their data available
		if (Manager->HaveAllFeaturesReachedInitState(const_cast<AZodiacHeroActor*>(this), InitState_DataAvailable))
		{
			UE_LOG_WITH_ROLE(LogZodiacFramework, Warning, TEXT("Hero actor success to change state to %s"), *DesiredState.ToString());
			return true;
		}
		else
		{
			UE_LOG_WITH_ROLE(LogZodiacFramework, Log, TEXT("Hero actor failed to change state to %s. Reason: other feature data is not available"), *DesiredState.ToString());
			return false;
		}
	}
	else if (CurrentState == InitState_DataInitialized && DesiredState == InitState_GameplayReady)
	{
		UE_LOG_WITH_ROLE(LogZodiacFramework, Warning, TEXT("Hero actor success to change state to %s"), *DesiredState.ToString());
		return true;
	}

	return false;
}

void AZodiacHeroActor::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	UE_LOG_WITH_ROLE(LogZodiacFramework, Warning, TEXT("Hero current state: %s, desired state: %s"), *CurrentState.ToString(), *DesiredState.ToString());

	if (CurrentState == InitState_Spawned && DesiredState == InitState_DataAvailable)
	{
		
	}
	else if (CurrentState == InitState_DataAvailable && DesiredState == InitState_DataInitialized)
	{
		SetAutonomousProxy(true);
		
		UZodiacHostAbilitySystemComponent* HostASC = HostCharacter->GetHostAbilitySystemComponent();
		AbilitySystemComponent->SetHostAbilitySystemComponent(HostASC);
		
		InitializeAbilitySystem();
		InitializeWithAbilitySystem();
	}
	else if (CurrentState == InitState_DataInitialized && DesiredState == InitState_GameplayReady)
	{
		InitializeMesh();
		AbilityManagerComponent->InitializeSlotTags();
	}
}

void AZodiacHeroActor::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature is now in DataAvailable, see if we should transition to DataInitialized
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		if (Params.FeatureState == InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
	else if (Params.FeatureName == NAME_ActorFeatureName)
	{
		UE_LOG_WITH_ROLE(LogZodiacFramework, Log, TEXT("Hero Actor Call Host CheckDefaultInit"));
		CheckHostDefaultInitializationNextTick();
	}
}

void AZodiacHeroActor::CheckDefaultInitialization()
{
	UE_LOG_WITH_ROLE(LogZodiacFramework, Log, TEXT("Hero CheckDefaultInit"));
	
	// Before checking our progress, try progressing any other features we might depend on
	CheckDefaultInitializationForImplementers();
	
	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

UAbilitySystemComponent* AZodiacHeroActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UZodiacAbilitySystemComponent* AZodiacHeroActor::GetZodiacAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UZodiacHeroAbilitySystemComponent* AZodiacHeroActor::GetHeroAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AZodiacHeroActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	DefaultMeshTransform = Mesh->GetRelativeTransform();	
}

void AZodiacHeroActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, bIsActive, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ThisClass, HostCharacter, COND_InitialOnly);
}

void AZodiacHeroActor::OnOwnerChanged()
{
	CheckDefaultInitialization();
}

void AZodiacHeroActor::CheckHostDefaultInitializationNextTick()
{
	if (HostCharacter.Get())
	{
		if (UZodiacHostExtensionComponent* HostExtensionComponent = HostCharacter->FindComponentByClass<UZodiacHostExtensionComponent>())
		{
			HostExtensionComponent->CheckDefaultInitialization();
		}	
		// GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		// 	{
		// 		if (UZodiacHostExtensionComponent* HostExtensionComponent = HostCharacter->FindComponentByClass<UZodiacHostExtensionComponent>())
		// 		{
		// 			HostExtensionComponent->CheckDefaultInitialization();
		// 		}	
		// 	});	
	}
}

void AZodiacHeroActor::OnRep_Owner()
{
	Super::OnRep_Owner();

	OnOwnerChanged();
}

void AZodiacHeroActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	Mesh->SetVisibility(false);

	// Register with the init state system early, this will only work if this is a game world
	RegisterInitStateFeature();
}

void AZodiacHeroActor::BeginPlay()
{
	Super::BeginPlay();

	// Listen for changes to all features
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);
	
	// Notifies state manager that we have spawned, then try the rest of default initialization
	ensure(TryToChangeInitState(InitState_Spawned));

	// if (HasAuthority())
	// {
	// 	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	// 	{
	// 		OnOwnerChanged();	
	// 	});
	// }

	//UE_LOG_WITH_ROLE(LogZodiacFramework, Log, TEXT("Hero Owner: %s"), *GetNameSafe(Owner));
}

void AZodiacHeroActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	UnregisterInitStateFeature();
}

void AZodiacHeroActor::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->GetOwnedGameplayTags(TagContainer);
	}
}

bool AZodiacHeroActor::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return ASC->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool AZodiacHeroActor::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return ASC->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool AZodiacHeroActor::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return ASC->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}

FGenericTeamId AZodiacHeroActor::GetGenericTeamId() const
{
	if (HostCharacter.Get())
	{
		return HostCharacter->GetGenericTeamId();
	}

	return FGenericTeamId::NoTeam;
}

bool AZodiacHeroActor::IsHostLocallyControlled() const
{
	if (HostCharacter.Get())
	{
		return HostCharacter->IsLocallyControlled();
	}

	return false;
}

USkeletalMeshComponent* AZodiacHeroActor::GetMesh() const
{
	return Mesh;
}

UZodiacHealthComponent* AZodiacHeroActor::GetHealthComponent() const
{
	return HealthComponent;
}

UZodiacHeroAnimInstance* AZodiacHeroActor::GetHeroAnimInstance() const
{
	if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
	{
		// Look for a linked anim layer first
		if (UZodiacHeroAnimInstance* HeroAnimInstance = Cast<UZodiacHeroAnimInstance>(AnimInstance->GetLinkedAnimLayerInstanceByClass(UZodiacHeroAnimInstance::StaticClass(), true)))
		{
			return HeroAnimInstance;
		}

		if (UZodiacHeroAnimInstance* HeroAnimInstance = Cast<UZodiacHeroAnimInstance>(AnimInstance))
		{
			return HeroAnimInstance;
		}
	}
	
	return nullptr;
}

void AZodiacHeroActor::SetHostCharacter(AZodiacHostCharacter* InHostCharacter)
{
	HostCharacter = InHostCharacter;
	
	if (HasAuthority())
	{
		OnRep_HostCharacter();
	}
}

AZodiacPlayerController* AZodiacHeroActor::GetHostController() const
{
	return  Cast<AZodiacPlayerController>(Owner);
}

void AZodiacHeroActor::InitializeAbilitySystem()
{
	check(AbilitySystemComponent);
	check(HostCharacter.Get())
	
	AbilitySystemComponent->InitAbilityActorInfo(HostCharacter.Get(), this);
	AbilitySystemComponent->RegisterGameplayTagEvent(Status_Focus, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStatusTagChanged);	
	AbilitySystemComponent->RegisterGameplayTagEvent(Status_ADS, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStatusTagChanged);	
	
	if (HeroData)
	{
		if (HasAuthority())
		{
			for (TObjectPtr<UZodiacAbilitySet> AbilitySet : HeroData->AbilitySets)
			{
				if (AbilitySet)
				{
					AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);	
				}
			}
		}
	}
}

void AZodiacHeroActor::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(Ability_Behavior_SurvivesDeath);

		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		AbilitySystemComponent->ClearAbilityInput();
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		else
		{
			// If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
			AbilitySystemComponent->ClearActorInfo();
		}
	}

	AbilitySystemComponent = nullptr;
}

void AZodiacHeroActor::InitializeWithAbilitySystem()
{
	AbilityManagerComponent->InitializeWithAbilitySystem(AbilitySystemComponent, HeroData);
	if (HostCharacter->IsLocallyControlled())
	{
		AbilityManagerComponent->BindMessageDelegates();	
	}
	
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);

	if (UZodiacHeroAnimInstance* HeroAnimInstance = GetHeroAnimInstance())
	{
		HeroAnimInstance->InitializeWithAbilitySystem(AbilitySystemComponent);
	}
}

void AZodiacHeroActor::InitializeMesh()
{
	check(HostCharacter.Get())
	
	// attach hero mesh to host mesh
	USkeletalMeshComponent* HostMesh = HostCharacter->GetMesh();
	check(HostMesh)
	
	AttachToComponent(HostMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, NAME_None);
	HostCharacter->OnCharacterAttached(this);
		
	// move up as much as UEFN mannequin height.
	// if (HasAuthority())
	// {
	// 	float HeightOffset = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	// 	AddActorLocalOffset(FVector(0, 0, HeightOffset));
	// }
	
	Mesh->AddTickPrerequisiteComponent(HostMesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->LinkAnimClassLayers(AnimLayerClass);
}

void AZodiacHeroActor::OnStatusTagChanged(FGameplayTag Tag, int Count)
{
	if (UZodiacHeroAnimInstance* HeroAnimInstance = GetHeroAnimInstance())
	{
		bool bHasTag = Count > 0;
		int32 NewCount = bHasTag ? 1 : 0;
	
		HeroAnimInstance->OnStatusChanged(Tag, bHasTag);	
	}
}

void AZodiacHeroActor::SetModularMesh(TSubclassOf<USkeletalMeshComponent> SkeletalMeshCompClass, FName Socket)
{
	if (ModularMeshComponent && ModularMeshComponent->IsRegistered())
	{
		ModularMeshComponent->UnregisterComponent();
	}
	USkeletalMeshComponent* NewMeshComp = NewObject<USkeletalMeshComponent>(this, SkeletalMeshCompClass);
	ModularMeshComponent = NewMeshComp;
	ModularMeshComponent->LeaderPoseComponent = Mesh;
	ModularMeshComponent->bUseBoundsFromLeaderPoseComponent = true;
	ModularMeshComponent->AddTickPrerequisiteComponent(Mesh);
	ModularMeshComponent->SetVisibility(false);
	ModularMeshComponent->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
	ModularMeshComponent->SetIsReplicated(false);
	ModularMeshComponent->RegisterComponent();
	ModularMeshComponent->GetAnimInstance()->LinkAnimClassLayers(AnimLayerClass);
	ModularMeshComponent->SetVisibility(true);
}

void AZodiacHeroActor::ClearModularMesh()
{
	if  (ModularMeshComponent)
	{
		ModularMeshComponent->SetVisibility(false);
		if (ModularMeshComponent->IsRegistered())
		{
			ModularMeshComponent->UnregisterComponent();	
		}
	}
}

void AZodiacHeroActor::Activate()
{
	bIsActive =  true;

	if (UZodiacHeroSkeletalMeshComponent* HeroMesh = Cast<UZodiacHeroSkeletalMeshComponent>(Mesh))
	{
		HeroMesh->bDisableClothSimulation = false;
		HeroMesh->ForceClothNextUpdateTeleportAndReset();
		HeroMesh->SetVisibility(true);
		HeroMesh->bIsHeroHidden = false;
		HeroMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeroMesh->SetRelativeTransform(DefaultMeshTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}
	
	if (HostCharacter.Get())
	{
		if (UZodiacCharacterMovementComponent* ZodiacCharMovComp = Cast<UZodiacCharacterMovementComponent>(HostCharacter->GetCharacterMovement()))
		{
			HostCharacter->SetExtendedMovementConfig(HeroData->ExtendedMovementConfig);

			bool bShouldStrafe = (HeroData->ExtendedMovementConfig.DefaultExtendedMovement != EZodiacExtendedMovementMode::Sprinting);
			ZodiacCharMovComp->ToggleStrafe(bShouldStrafe);
		}
	}
	
	OnHeroActivated.Broadcast();
	OnHeroActivated_BP.Broadcast(this);
}

void AZodiacHeroActor::Deactivate()
{
	if (UZodiacHeroSkeletalMeshComponent* HeroMesh = Cast<UZodiacHeroSkeletalMeshComponent>(Mesh))
	{
		HeroMesh->SetVisibility(false);
		HeroMesh->bIsHeroHidden = true;
		HeroMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HeroMesh->bDisableClothSimulation = true;
	}
	
	bIsActive = false;

	OnHeroDeactivated.Broadcast();
	OnHeroDeactivated_BP.Broadcast(this);
}

void AZodiacHeroActor::OnRep_HostCharacter()
{
	CheckDefaultInitialization();
}

void AZodiacHeroActor::OnRep_IsActive(bool OldValue)
{
	if (bIsActive != OldValue)
	{
		bIsActive ? Activate() : Deactivate();
	}
}

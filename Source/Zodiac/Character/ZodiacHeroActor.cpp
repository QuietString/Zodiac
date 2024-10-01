// the.quiet.string@gmail.com

#include "ZodiacHeroActor.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacHeroData.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHostCharacter.h"
#include "AbilitySystem/ZodiacHeroAbilitySystemComponent.h"
#include "Animation/ZodiacHeroAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "ZodiacHUDManagerComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroActor)

AZodiacHeroActor::AZodiacHeroActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	
	Mesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("Mesh"));
	Mesh->AlwaysLoadOnClient = true;
	Mesh->AlwaysLoadOnServer = false;
	Mesh->bOwnerNoSee = false;
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh->bCastDynamicShadow = true;
	Mesh->bAffectDynamicIndirectLighting = true;
	Mesh->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCanEverAffectNavigation(false);
	Mesh->CanCharacterStepUpOn = ECB_No;
	Mesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Mesh->SetVisibility(false);
	RootComponent = Mesh;
	
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHeroAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHealthComponent>(this, TEXT("HealthComponent"));

	HUDManagerComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHUDManagerComponent>(this, TEXT("HUD Manager"));
}

void AZodiacHeroActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, bIsActive, COND_InitialOnly);
}

void AZodiacHeroActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (HasAuthority())
	{
		Initialize();
	}
}

void AZodiacHeroActor::BeginPlay()
{
	Super::BeginPlay();
}

void AZodiacHeroActor::OnRep_Owner()
{
	Initialize();
}

FGenericTeamId AZodiacHeroActor::GetGenericTeamId() const
{
	if (HostCharacter)
	{
		return HostCharacter->GetGenericTeamId();
	}

	return FGenericTeamId::NoTeam;
}

UAbilitySystemComponent* AZodiacHeroActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UZodiacAbilitySystemComponent* AZodiacHeroActor::GetHeroAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

AZodiacHostCharacter* AZodiacHeroActor::GetHostCharacter() const
{
	return HostCharacter;
}

void AZodiacHeroActor::InitializeAbilitySystem()
{
	check(AbilitySystemComponent);

	AbilitySystemComponent->InitAbilityActorInfo(Owner, this);
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_Focus, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStatusTagChanged);	
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_WeaponReady, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStatusTagChanged);

	if (HeroData && HasAuthority())
	{
		for (TObjectPtr<UZodiacAbilitySet> AbilitySet : HeroData->AbilitySets)
		{
			if (AbilitySet)
			{
				AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);	
			}
		}
	}
	
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
}

void AZodiacHeroActor::OnStatusTagChanged(FGameplayTag Tag, int Count)
{
	check(HostCharacter);
	
	if (UAbilitySystemComponent* HostASC = HostCharacter->GetHostAbilitySystemComponent())
	{
		bool bHasTag = Count > 0;
		int32 NewCount = bHasTag ? 1 : 0;
		
		HostASC->SetLooseGameplayTagCount(Tag, NewCount);
		
		if (UZodiacHeroAnimInstance* HeroAnimInstance = Cast<UZodiacHeroAnimInstance>(Mesh->GetAnimInstance()))
		{
			HeroAnimInstance->OnStatusChanged(Tag, bHasTag);
		}
	}
}

UZodiacHealthComponent* AZodiacHeroActor::GetHealthComponent() const
{
	return HealthComponent;
}

void AZodiacHeroActor::Activate()
{
	bIsActive =  true;

	Mesh->SetVisibility(true);
	if (HostCharacter)
	{
		if (UCapsuleComponent* Capsule = HostCharacter->GetCapsuleComponent())
		{
			if (USkeletalMeshComponent* HostMesh = HostCharacter->GetMesh())
			{
				// Prevent character feet floating when capsule height increases.
				//float OffsetAmount = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - Capsule->GetUnscaledCapsuleHalfHeight();
				//HostMesh->AddRelativeLocation(FVector(0.0f, 0.0f, -OffsetAmount));
			}
			
			//Capsule->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
		}
		
		HostCharacter->TryChangeMovementMode(MOVE_Walking, HeroData->DefaultMovementMode);
	}
	
	OnHeroActivated.Broadcast();

	
	// @TODO: change tick option for optimization.
	//Mesh->VisibilityBasedAnimTickOption = 
}

void AZodiacHeroActor::Deactivate()
{
	Mesh->SetVisibility(false);
	bIsActive = false;
}

void AZodiacHeroActor::Initialize()
{
	HostCharacter = Cast<AZodiacHostCharacter>(Owner);
	if (HostCharacter)
	{
		HostCharacter->CallOrRegister_OnAbilitySystemInitialized(FOnAbilitySystemComponentInitialized::FDelegate::CreateUObject(this, &ThisClass::OnHostAbilitySystemComponentInitialized));
		
		AttachToOwner();
		InitializeAbilitySystem();
	}
}

void AZodiacHeroActor::AttachToOwner()
{
	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		if (USkeletalMeshComponent* CharacterMesh = Character->GetMesh())
		{
			AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, NAME_None);
			AddTickPrerequisiteComponent(CharacterMesh);
		}
	}
}

void AZodiacHeroActor::OnHostAbilitySystemComponentInitialized(UAbilitySystemComponent* HostASC)
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = Cast<UZodiacAbilitySystemComponent>(HostASC))
	{
		AbilitySystemComponent->SetHostAbilitySystemComponent(ZodiacASC);	
	}
}

void AZodiacHeroActor::OnRep_bIsActive(bool OldValue)
{
	if (bIsActive != OldValue)
	{
		bIsActive ? Activate() : Deactivate();
	}
}

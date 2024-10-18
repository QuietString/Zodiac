// the.quiet.string@gmail.com

#include "ZodiacHeroCharacter.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacHeroData.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHostCharacter.h"
#include "AbilitySystem/ZodiacHeroAbilitySystemComponent.h"
#include "Animation/ZodiacHeroAnimInstance.h"
#include "ZodiacHeroAbilityManagerComponent.h"
#include "ZodiacHeroSkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroCharacter)

AZodiacHeroCharacter::AZodiacHeroCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZodiacHeroSkeletalMeshComponent>(MeshComponentName))
{
	bReplicates = true;

	UZodiacHeroSkeletalMeshComponent* HeroMesh = CastChecked<UZodiacHeroSkeletalMeshComponent>(GetMesh());
	HeroMesh->bIsHeroHidden = true;
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHeroAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHealthComponent>(this, TEXT("HealthComponent"));
	
	AbilityManagerComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHeroAbilityManagerComponent>(this, TEXT("Ability Manager"));
	AbilityManagerComponent->SetComponentTickEnabled(true);
}

void AZodiacHeroCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, bIsActive, COND_InitialOnly);
}

void AZodiacHeroCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (HasAuthority())
	{
		InitializeWithHostCharacter();
	}
}

void AZodiacHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetMesh()->LinkAnimClassLayers(AnimLayerClass);
}

void AZodiacHeroCharacter::OnRep_Owner()
{
	InitializeWithHostCharacter();
}

FGenericTeamId AZodiacHeroCharacter::GetGenericTeamId() const
{
	if (HostCharacter)
	{
		return HostCharacter->GetGenericTeamId();
	}

	return FGenericTeamId::NoTeam;
}

UAbilitySystemComponent* AZodiacHeroCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UZodiacAbilitySystemComponent* AZodiacHeroCharacter::GetHeroAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UZodiacAbilitySystemComponent* AZodiacHeroCharacter::GetHostAbilitySystemComponent() const
{
	if (HostCharacter)
	{
		return HostCharacter->GetHostAbilitySystemComponent();
	}

	return nullptr;
}

AZodiacHostCharacter* AZodiacHeroCharacter::GetHostCharacter() const
{
	return HostCharacter;
}

void AZodiacHeroCharacter::InitializeAbilitySystem()
{
	check(AbilitySystemComponent);

	AbilitySystemComponent->InitAbilityActorInfo(Owner, this);
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_Focus, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStatusTagChanged);	
	//AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_WeaponReady, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStatusTagChanged);

	if (HeroData)
	{
		AbilityManagerComponent->InitializeWithAbilitySystem(AbilitySystemComponent, HeroData);

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
	
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
}

void AZodiacHeroCharacter::OnStatusTagChanged(FGameplayTag Tag, int Count)
{
	check(HostCharacter);
	
	if (UAbilitySystemComponent* HostASC = HostCharacter->GetHostAbilitySystemComponent())
	{
		bool bHasTag = Count > 0;
		int32 NewCount = bHasTag ? 1 : 0;
		
		HostASC->SetLooseGameplayTagCount(Tag, NewCount);
	}
}

UZodiacHealthComponent* AZodiacHeroCharacter::GetHealthComponent() const
{
	return HealthComponent;
}

UZodiacHeroAnimInstance* AZodiacHeroCharacter::GetHeroAnimInstance() const
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		// Look for linked anim layer first
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

void AZodiacHeroCharacter::SetModularMesh(TSubclassOf<USkeletalMeshComponent> SkeletalMeshCompClass, FName Socket)
{
	if (ModularMeshComponent)
	{
		ModularMeshComponent->UnregisterComponent();
	}
	
	USkeletalMeshComponent* NewMeshComp = NewObject<USkeletalMeshComponent>(this, SkeletalMeshCompClass);
	ModularMeshComponent = NewMeshComp;
	ModularMeshComponent->LeaderPoseComponent = GetMesh();
	ModularMeshComponent->bUseBoundsFromLeaderPoseComponent = true;
	ModularMeshComponent->AddTickPrerequisiteComponent(GetMesh());
	ModularMeshComponent->SetVisibility(false);
	ModularMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
	ModularMeshComponent->SetIsReplicated(false);
	ModularMeshComponent->RegisterComponent();
	ModularMeshComponent->GetAnimInstance()->LinkAnimClassLayers(AnimLayerClass);
	ModularMeshComponent->SetVisibility(true);
}

void AZodiacHeroCharacter::ClearModularMesh()
{
	if  (ModularMeshComponent)
	{
		ModularMeshComponent->SetVisibility(false);
		ModularMeshComponent->UnregisterComponent();	
	}
}

void AZodiacHeroCharacter::Activate()
{
	bIsActive =  true;

	if (UZodiacHeroSkeletalMeshComponent* HeroMesh = Cast<UZodiacHeroSkeletalMeshComponent>(GetMesh()))
	{
		HeroMesh->SetVisibility(true);
		HeroMesh->bIsHeroHidden = false;
	}
	
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
		HostCharacter->SetMovementSpeeds(HeroData->WalkSpeeds, HeroData->RunSpeeds);
	}
	
	OnHeroActivated.Broadcast();
}

void AZodiacHeroCharacter::Deactivate()
{
	if (UZodiacHeroSkeletalMeshComponent* HeroMesh = Cast<UZodiacHeroSkeletalMeshComponent>(GetMesh()))
	{
		HeroMesh->SetVisibility(false);
		HeroMesh->bIsHeroHidden = true;
	}
	
	bIsActive = false;

	OnHeroDeactivated.Broadcast();
}

void AZodiacHeroCharacter::InitializeWithHostCharacter()
{
	HostCharacter = Cast<AZodiacHostCharacter>(Owner);
	if (HostCharacter)
	{
		HostCharacter->CallOrRegister_OnAbilitySystemInitialized(FOnAbilitySystemComponentInitialized::FDelegate::CreateUObject(this, &ThisClass::OnHostAbilitySystemComponentInitialized));
		
		AttachToOwner();
		InitializeAbilitySystem();

		GetMesh()->AddTickPrerequisiteComponent(HostCharacter->GetMesh());
	}
}

void AZodiacHeroCharacter::AttachToOwner()
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

void AZodiacHeroCharacter::OnHostAbilitySystemComponentInitialized(UAbilitySystemComponent* HostASC)
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = Cast<UZodiacAbilitySystemComponent>(HostASC))
	{
		AbilitySystemComponent->SetHostAbilitySystemComponent(ZodiacASC);	
	}
}

void AZodiacHeroCharacter::OnRep_bIsActive(bool OldValue)
{
	if (bIsActive != OldValue)
	{
		bIsActive ? Activate() : Deactivate();
	}
}

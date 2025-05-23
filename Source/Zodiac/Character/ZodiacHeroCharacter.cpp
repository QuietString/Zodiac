﻿// the.quiet.string@gmail.com

#include "ZodiacHeroCharacter.h"

#include "ZodiacCharacterMovementComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHeroData.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHostCharacter.h"
#include "AbilitySystem/Hero/ZodiacHeroAbilitySystemComponent.h"
#include "Animation/ZodiacHeroAnimInstance.h"
#include "ZodiacHeroAbilityManagerComponent.h"
#include "ZodiacHeroSkeletalMeshComponent.h"
#include "AbilitySystem/Host/ZodiacHostAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroCharacter)

AZodiacHeroCharacter::AZodiacHeroCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZodiacHeroSkeletalMeshComponent>(MeshComponentName))
{
	bReplicates = true;

	// We are driving a character from AHostCharacter.
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->PrimaryComponentTick.bCanEverTick = false;
	GetCharacterMovement()->PrimaryComponentTick.bStartWithTickEnabled = false;
	
	UZodiacHeroSkeletalMeshComponent* HeroMesh = CastChecked<UZodiacHeroSkeletalMeshComponent>(GetMesh());
	HeroMesh->bIsHeroHidden = true;
	HeroMesh->ClothTeleportMode = EClothingTeleportMode::TeleportAndReset;
	
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

void AZodiacHeroCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	DefaultMeshTransform = GetMesh()->GetRelativeTransform();
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
}

void AZodiacHeroCharacter::OnRep_Owner()
{
	InitializeWithHostCharacter();
}

void AZodiacHeroCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->GetOwnedGameplayTags(TagContainer);
	}
}

bool AZodiacHeroCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return ASC->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool AZodiacHeroCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return ASC->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool AZodiacHeroCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return ASC->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
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

UZodiacAbilitySystemComponent* AZodiacHeroCharacter::GetZodiacAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UZodiacHeroAbilitySystemComponent* AZodiacHeroCharacter::GetHeroAbilitySystemComponent() const
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
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_ADS, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStatusTagChanged);	
	
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

	GetMesh()->LinkAnimClassLayers(AnimLayerClass);
	
	if (UZodiacHeroAnimInstance* HeroAnimInstance = GetHeroAnimInstance())
	{
		HeroAnimInstance->InitializeWithAbilitySystem(AbilitySystemComponent);
	}
	
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);

	bIsInitialized = true;
	GetHostCharacter()->CheckAllHeroesInitialized();
}

void AZodiacHeroCharacter::OnStatusTagChanged(FGameplayTag Tag, int Count)
{
	if (UZodiacHeroAnimInstance* HeroAnimInstance = GetHeroAnimInstance())
	{
		bool bHasTag = Count > 0;
		int32 NewCount = bHasTag ? 1 : 0;
	
		HeroAnimInstance->OnStatusChanged(Tag, bHasTag);	
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
	if (ModularMeshComponent && ModularMeshComponent->IsRegistered())
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
		if (ModularMeshComponent->IsRegistered())
		{
			ModularMeshComponent->UnregisterComponent();	
		}
	}
}

void AZodiacHeroCharacter::Activate()
{
	bIsActive =  true;

	if (UZodiacHeroSkeletalMeshComponent* HeroMesh = Cast<UZodiacHeroSkeletalMeshComponent>(GetMesh()))
	{
		HeroMesh->bDisableClothSimulation = false;
		HeroMesh->ForceClothNextUpdateTeleportAndReset();
		HeroMesh->SetVisibility(true);
		HeroMesh->bIsHeroHidden = false;
		HeroMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeroMesh->SetRelativeTransform(DefaultMeshTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}
	
	if (HostCharacter)
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

void AZodiacHeroCharacter::Deactivate()
{
	if (UZodiacHeroSkeletalMeshComponent* HeroMesh = Cast<UZodiacHeroSkeletalMeshComponent>(GetMesh()))
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

void AZodiacHeroCharacter::InitializeWithHostCharacter()
{
	HostCharacter = Cast<AZodiacHostCharacter>(Owner);
	if (HostCharacter)
	{
		HostCharacter->CallOrRegister_OnAbilitySystemInitialized(FOnAbilitySystemComponentInitialized::FDelegate::CreateUObject(this, &ThisClass::OnHostAbilitySystemComponentInitialized));
		
		AttachToOwner();
		InitializeAbilitySystem();

		GetMesh()->AddTickPrerequisiteComponent(HostCharacter->GetMesh());
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (HostCharacter->IsLocallyControlled())
		{
			AbilityManagerComponent->BindMessageDelegates();	
		}
	}
}

void AZodiacHeroCharacter::AttachToOwner()
{
	if (AZodiacHostCharacter* OwnerHostCharacter = Cast<AZodiacHostCharacter>(Owner))
	{
		if (USkeletalMeshComponent* HostMesh = OwnerHostCharacter->GetMesh())
		{
			AttachToComponent(HostMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, NAME_None);
			OwnerHostCharacter->OnCharacterAttached(this);
			
			// move up as much as UEFN mannequin height.
			if (HasAuthority())
			{
				float HeightOffset = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
				AddActorLocalOffset(FVector(0, 0, HeightOffset));
			}
			AddTickPrerequisiteComponent(HostMesh);
		}
	}
}

void AZodiacHeroCharacter::OnHostAbilitySystemComponentInitialized(UAbilitySystemComponent* InHostASC)
{
	if (UZodiacHostAbilitySystemComponent* HostASC = Cast<UZodiacHostAbilitySystemComponent>(InHostASC))
	{
		AbilitySystemComponent->SetHostAbilitySystemComponent(HostASC);	
	}
}

void AZodiacHeroCharacter::OnRep_IsActive(bool OldValue)
{
	if (bIsActive != OldValue)
	{
		bIsActive ? Activate() : Deactivate();
	}
}

// the.quiet.string@gmail.com

#include "ZodiacHero.h"

#include "AbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHeroData.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHostCharacter.h"
#include "Animation/ZodiacHeroAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHero)

AZodiacHero::AZodiacHero(const FObjectInitializer& ObjectInitializer)
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
	
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHealthComponent>(this, TEXT("HealthComponent"));
}

void AZodiacHero::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, bIsActive, COND_InitialOnly);
}

void AZodiacHero::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (HasAuthority())
	{
		Initialize();
	}
}

void AZodiacHero::BeginPlay()
{
	Super::BeginPlay();
}

void AZodiacHero::OnRep_Owner()
{
	Initialize();
}

UAbilitySystemComponent* AZodiacHero::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

TObjectPtr<UZodiacAbilitySystemComponent> AZodiacHero::GetZodiacAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

AZodiacHostCharacter* AZodiacHero::GetHostCharacter() const
{
	return HostCharacter;
}

void AZodiacHero::InitializeAbilitySystem()
{
	check(AbilitySystemComponent);

	AbilitySystemComponent->InitAbilityActorInfo(Owner, this);
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Movement_Mode_Walking_Aiming, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnMovementTagChanged);	
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_WeaponReady, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnMovementTagChanged);	

	if (HeroData && HasAuthority())
	{
		for (TObjectPtr<UZodiacAbilitySet> AbilitySet : HeroData->AbilitySets)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}
	
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
}

void AZodiacHero::OnMovementTagChanged(FGameplayTag Tag, int Count)
{
	check(HostCharacter);
	
	if (UAbilitySystemComponent* HostASC = HostCharacter->GetAbilitySystemComponent())
	{
		if (Count > 0)
		{
			HostASC->SetLooseGameplayTagCount(Tag, 1);
		}
		else
		{
			HostASC->SetLooseGameplayTagCount(Tag, 0);
		}

		UZodiacHeroAnimInstance* HeroAnimInstance = CastChecked<UZodiacHeroAnimInstance>(Mesh->GetAnimInstance());
		if (Tag == ZodiacGameplayTags::Movement_Mode_Walking_Aiming)
		{
			(Count > 0) ? HeroAnimInstance->OnAimingChanged(true) : HeroAnimInstance->OnAimingChanged(false); 
		}
		else if (Tag == ZodiacGameplayTags::Status_WeaponReady)
		{
			(Count > 0) ? HeroAnimInstance->OnIsPistolReadyChanged(true) : HeroAnimInstance->OnIsPistolReadyChanged(false);
		}
	}
}

UZodiacHealthComponent* AZodiacHero::GetHealthComponent() const
{
	return HealthComponent;
}

void AZodiacHero::Activate()
{
	Mesh->SetVisibility(true);
	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
		{
			Capsule->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
		}
	}
 	bIsActive =  true;

	// @TODO: change tick option for optimization.
	//Mesh->VisibilityBasedAnimTickOption = 
}

void AZodiacHero::Deactivate()
{
	Mesh->SetVisibility(false);
	bIsActive = false;
}

void AZodiacHero::Initialize()
{
	HostCharacter = Cast<AZodiacHostCharacter>(Owner);
	if (HostCharacter)
	{
		HostCharacter->CallOrRegister_OnAbilitySystemInitialized(FOnHostAbilitySystemComponentLoaded::FDelegate::CreateUObject(this, &ThisClass::OnHostAbilitySystemComponentInitialized));
		
		AttachToOwner();
		InitializeAbilitySystem();
	}
}

void AZodiacHero::AttachToOwner()
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

void AZodiacHero::OnHostAbilitySystemComponentInitialized(UAbilitySystemComponent* HostASC)
{
	if (UZodiacHeroAnimInstance* HeroAnimInstance = Cast<UZodiacHeroAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		HeroAnimInstance->InitializeWithAbilitySystem(HostASC);
	}
}

void AZodiacHero::OnRep_bIsActive(bool OldValue)
{
	if (bIsActive != OldValue)
	{
		bIsActive ? Activate() : Deactivate();
	}
}

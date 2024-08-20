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
#include "AbilitySystem/ZodiacHeroAbilitySystemComponent.h"
#include "Animation/ZodiacHeroAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "ZodiacHUDManagerComponent.h"
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
	
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHeroAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHealthComponent>(this, TEXT("HealthComponent"));

	HUDManagerComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHUDManagerComponent>(this, TEXT("HUD Manager"));
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

UZodiacAbilitySystemComponent* AZodiacHero::GetHeroAbilitySystemComponent() const
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

void AZodiacHero::OnStatusTagChanged(FGameplayTag Tag, int Count)
{
	check(HostCharacter);
	
	if (UAbilitySystemComponent* HostASC = HostCharacter->GetHostAbilitySystemComponent())
	{
		bool bHasTag = Count > 0;
		int32 NewCount = bHasTag ? 1 : 0;
		
		HostASC->SetLooseGameplayTagCount(Tag, NewCount);
		
		UZodiacHeroAnimInstance* HeroAnimInstance = CastChecked<UZodiacHeroAnimInstance>(Mesh->GetAnimInstance());
		HeroAnimInstance->OnStatusChanged(Tag, bHasTag);
	}
}

UZodiacHealthComponent* AZodiacHero::GetHealthComponent() const
{
	return HealthComponent;
}

void AZodiacHero::Activate()
{
	bIsActive =  true;

	Mesh->SetVisibility(true);
	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
		{
			Capsule->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
		}
	}
	
	OnHeroActivated.Broadcast();

	
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
		HostCharacter->CallOrRegister_OnAbilitySystemInitialized(FOnAbilitySystemComponentInitialized::FDelegate::CreateUObject(this, &ThisClass::OnHostAbilitySystemComponentInitialized));
		
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
	if (UZodiacAbilitySystemComponent* ZodiacASC = Cast<UZodiacAbilitySystemComponent>(HostASC))
	{
		AbilitySystemComponent->SetHostAbilitySystemComponent(ZodiacASC);	
	}
	
	if (UZodiacHeroAnimInstance* HeroAnimInstance = Cast<UZodiacHeroAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		HeroAnimInstance->InitializeWithAbilitySystem(AbilitySystemComponent);
	}
}

void AZodiacHero::OnRep_bIsActive(bool OldValue)
{
	if (bIsActive != OldValue)
	{
		bIsActive ? Activate() : Deactivate();
	}
}

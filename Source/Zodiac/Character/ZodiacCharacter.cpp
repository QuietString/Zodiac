// the.quiet.string@gmail.com


#include "ZodiacCharacter.h"

#include "ZodiacHealthComponent.h"
#include "ZodiacHeroComponent.h"
#include "ZodiacLogChannels.h"
#include "ZodiacPawnExtensionComponent.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"
#include "Player/ZodiacPlayerState.h"


AZodiacCharacter::AZodiacCharacter(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	PawnExtComponent = CreateDefaultSubobject<UZodiacPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->RegisterAndCall_OnAbilitySystemInitialized(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));

	HealthComponent = CreateDefaultSubobject<UZodiacHealthComponent>(TEXT("HealthComponent"));

	HeroComponent = CreateDefaultSubobject<UZodiacHeroComponent>(TEXT("HeroComponent"));
}

AZodiacPlayerState* AZodiacCharacter::GetZodiacPlayerState() const
{
	return CastChecked<AZodiacPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

UZodiacAbilitySystemComponent* AZodiacCharacter::GetZodiacAbilitySystemComponent() const
{
	return Cast<UZodiacAbilitySystemComponent>(GetAbilitySystemComponent());
}

void AZodiacCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AZodiacCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtComponent->CheckPawnReadyToInitialize();
}

void AZodiacCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtComponent->CheckPawnReadyToInitialize();
}

void AZodiacCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AZodiacCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AZodiacCharacter::AddDefaultAbilities(UZodiacAbilitySystemComponent* ZodiacASC)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC && DefaultAbilitySpecs.Num() > 0)
	{
		for (auto& AbilitySpec : DefaultAbilitySpecs)
		{
			if (AbilitySpec.Ability && AbilitySpec.Level > 0)
			{
				ASC->GiveAbility(AbilitySpec);	
			}
			else
			{
				UE_LOG(LogZodiacAbilitySystem, Warning, TEXT("Attempted to give an invalid ability spec."));
			}
		}
	}
}

UAbilitySystemComponent* AZodiacCharacter::GetAbilitySystemComponent() const
{
	AZodiacPlayerState* ZodiacPS = GetZodiacPlayerState();
	return ZodiacPS->GetZodiacAbilitySystemComponent();
}

void AZodiacCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		ZodiacASC->GetOwnedGameplayTags(TagContainer);
	}
}

bool AZodiacCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (const UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		return ZodiacASC->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool AZodiacCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		return ZodiacASC->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool AZodiacCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		return ZodiacASC->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}

void AZodiacCharacter::OnAbilitySystemInitialized()
{
	UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent();
	check(ZodiacASC);

	HealthComponent->InitializeWithAbilitySystem(ZodiacASC);
}

void AZodiacCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	PawnExtComponent->CheckPawnReadyToInitialize();
	// AZodiacPlayerState* ZodiacPS = NewController->GetPlayerState<AZodiacPlayerState>();
	// check(ZodiacPS);
	//
	// AbilitySystemComponent = ZodiacPS->GetZodiacAbilitySystemComponent();
	//
	// OnAbilitySystemInitialized(AbilitySystemComponent);
	// HealthComponent->OnAbilitySystemInitialized(AbilitySystemComponent);
}

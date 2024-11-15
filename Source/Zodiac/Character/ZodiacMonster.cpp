// the.quiet.string@gmail.com


#include "ZodiacMonster.h"

#include "ZodiacAIController.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHeroData.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacMonster)

AZodiacMonster::AZodiacMonster(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHealthComponent>(this, TEXT("HealthComponent"));
	PhysicalAnimationComponent = ObjectInitializer.CreateDefaultSubobject<UPhysicalAnimationComponent>(this, TEXT("PhysicalAnimationComponent"));
}

UZodiacAbilitySystemComponent* AZodiacMonster::GetZodiacAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAbilitySystemComponent* AZodiacMonster::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UZodiacHealthComponent* AZodiacMonster::GetHealthComponent() const
{
	return HealthComponent;
}

FGenericTeamId AZodiacMonster::GetGenericTeamId() const
{
	if (AZodiacAIController* ZodiacAC = GetController<AZodiacAIController>())
	{
		return ZodiacAC->GetGenericTeamId();
	}
	
	return static_cast<uint8>(EZodiacTeam::NoTeam);
}

void AZodiacMonster::BeginPlay()
{
	Super::BeginPlay();

	if (HeroData && HasAuthority())
	{
		SetMovementMode(MOVE_Walking, HeroData->DefaultMovementMode);
		SetDefaultCustomMovementMode(HeroData->DefaultMovementMode);
	}
}

void AZodiacMonster::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitializeAbilitySystem(AbilitySystemComponent, this);
}

void AZodiacMonster::InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner)
{
	Super::InitializeAbilitySystem(InASC, InOwner);

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

// the.quiet.string@gmail.com

#include "ZodiacHeroComponent.h"

#include "ZodiacPlayerCharacter.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacHealthComponent.h"
#include "AbilitySystem/Attributes/ZodiacCombatSet.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"

UZodiacHeroComponent::UZodiacHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHealthComponent>(this, TEXT("HealthComponent"));
}

UZodiacAbilitySystemComponent* UZodiacHeroComponent::GetZodiacAbilitySystemComponent()
{
	return AbilitySystemComponent;
}

UAbilitySystemComponent* UZodiacHeroComponent::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void UZodiacHeroComponent::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetOwnedGameplayTags(TagContainer);	
	}
}

void UZodiacHeroComponent::OnRegister()
{
	if (HeroData)
	{
		// DefaultStartingData is added to ASC on OnRegister()
		AbilitySystemComponent->DefaultStartingData = HeroData->Attributes;
		HeroName = HeroData->HeroName;
	}
	
	Super::OnRegister();
}

void UZodiacHeroComponent::BeginPlay()
{
	Super::BeginPlay();
	
	ensureMsgf(HeroData, TEXT("Must have HeroData"));
	
	OnHeroChanged.Broadcast(this);
}

UZodiacAbilitySystemComponent* UZodiacHeroComponent::InitializeAbilitySystem()
{
	AddAbilities();
	AbilitySystemComponent->InitAbilityActorInfo(GetOwner(), GetOwner());

	const UAttributeSet* HealthSet_ASC = AbilitySystemComponent->GetAttributeSet(UZodiacHealthSet::StaticClass());
	HealthSet = Cast<UZodiacHealthSet>(HealthSet_ASC);
	check(HealthSet);
	
	const UAttributeSet* CombatSet_ASC = AbilitySystemComponent->GetAttributeSet(UZodiacCombatSet::StaticClass());
	CombatSet = Cast<UZodiacCombatSet>(CombatSet_ASC);
	check(CombatSet);
	
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);

	return AbilitySystemComponent;
}

void UZodiacHeroComponent::ActivateHero()
{
	OnHeroChanged.Broadcast(this);
}

void UZodiacHeroComponent::DeactivateHero()
{
	
}

void UZodiacHeroComponent::AddAbilities()
{
	if (HeroData && HeroData->AbilitySets.Num() > 0)
	{
		for (TObjectPtr<UZodiacAbilitySet> AbilitySet : HeroData->AbilitySets)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}
}

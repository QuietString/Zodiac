// the.quiet.string@gmail.com

#include "ZodiacHeroComponent.h"

#include "ZodiacPlayerCharacter.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHeroData.h"
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

TArray<FName> UZodiacHeroComponent::GetCurrentAbilitySockets(const FGameplayTag AbilityTag)
{
	for (auto& SocketSet : HeroData->SkillSockets)
	{
		if (SocketSet->SkillTag == AbilityTag)
		{
			return SocketSet->Sockets;
		}
	}
	
	return TArray<FName>();
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
	
	check(HeroData && "Hust have HeroData");
	
	OnHeroChanged.Broadcast(this);
}

UZodiacAbilitySystemComponent* UZodiacHeroComponent::InitializeAbilitySystem()
{
	AActor* Owner = GetOwner();
	check(Owner && "No Owner Actor");

	AddAbilities();
	
	AbilitySystemComponent->InitAbilityActorInfo(GetOwner(), GetOwner());
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

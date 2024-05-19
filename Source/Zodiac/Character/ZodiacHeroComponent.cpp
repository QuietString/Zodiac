// the.quiet.string@gmail.com

#include "ZodiacHeroComponent.h"

#include "HeroDisplayManagerComponent.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHeroData.h"
#include "AbilitySystem/Skills/ZodiacSkillDefinition.h"

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

bool UZodiacHeroComponent::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasMatchingGameplayTag(TagToCheck);
	}
	return IGameplayTagAssetInterface::HasMatchingGameplayTag(TagToCheck);
}

bool UZodiacHeroComponent::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool UZodiacHeroComponent::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
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

void UZodiacHeroComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UZodiacHeroComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UZodiacHeroComponent::InitializeDisplayManager()
{
	DisplayManager = NewObject<UHeroDisplayManagerComponent>(this);
	DisplayManager->InitializeHeroData(SlotIndex, AbilitySystemComponent, HeroData->SkillDefinitions, OnHeroChanged_Simple);
}

UZodiacAbilitySystemComponent* UZodiacHeroComponent::InitializeAbilitySystem()
{
	APawn* Pawn = GetPawn<APawn>();
	check(Pawn && "No Owner Actor");

	AddAbilities();
	
	AbilitySystemComponent->InitAbilityActorInfo(GetOwner(), GetOwner());
	HealthComponent->InitializeWithAbilitySystem(SlotIndex, AbilitySystemComponent);
	
	return AbilitySystemComponent;
}

void UZodiacHeroComponent::ActivateHero()
{
	OnHeroChanged.Broadcast(this);
	OnHeroChanged_Simple.Broadcast();
}

void UZodiacHeroComponent::DeactivateHero()
{
}

void UZodiacHeroComponent::AddAbilities()
{
	if (HeroData)
	{
		if (HeroData->AbilitySets.Num() >0)
		{
			for (TObjectPtr<UZodiacAbilitySet> AbilitySet : HeroData->AbilitySets)
			{
				AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
			}
		}
		
		if (HeroData->SkillDefinitions.Num() > 0)
		{
			for (auto& Skill : HeroData->SkillDefinitions)
			{
				Skill->GiveToAbilitySystemComponent(AbilitySystemComponent);
			}
		}
	}
}

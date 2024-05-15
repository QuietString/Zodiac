// the.quiet.string@gmail.com

#include "ZodiacHeroComponent.h"

#include "HeroDisplayManagerComponent.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHeroData.h"
#include "Skills/ZodiacSkillManagerComponent.h"

UZodiacHeroComponent::UZodiacHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHealthComponent>(this, TEXT("HealthComponent"));
	DisplayManager = CreateDefaultSubobject<UHeroDisplayManagerComponent>(TEXT("HeroDisplayManagerComponent"));
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

	return false;}

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
	check(HeroData && "Must have HeroData");

	Super::BeginPlay();
}

UZodiacAbilitySystemComponent* UZodiacHeroComponent::InitializeAbilitySystem()
{
	AActor* Owner = GetOwner();
	check(Owner && "No Owner Actor");

	AddAbilities();
	
	AbilitySystemComponent->InitAbilityActorInfo(GetOwner(), GetOwner());
	HealthComponent->InitializeWithAbilitySystem(SlotIndex, AbilitySystemComponent);
	DisplayManager->InitializeHeroData(SlotIndex, AbilitySystemComponent);
	OnHeroChanged_Simple.AddUObject(DisplayManager, &UHeroDisplayManagerComponent::OnHeroChanged);
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
	UE_LOG(LogTemp, Warning, TEXT("add abilities"));
	if (HeroData && HeroData->AbilitySets.Num() > 0)
	{
		for (TObjectPtr<UZodiacAbilitySet> AbilitySet : HeroData->AbilitySets)
		{
			// @TODO: can't get SKillData since it give abilities only on the server. 
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, OUT &AbilityHandles, OUT &SkillData);
		}
	}
	
	if (APawn* Pawn = GetPawn<APawn>())
	{
		DisplayManager->RegisterSkillDisplayData(SkillData);
	}
}

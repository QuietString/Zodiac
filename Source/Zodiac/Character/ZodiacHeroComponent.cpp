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

	//HealthSet = CreateDefaultSubobject<UZodiacHealthSet>(TEXT("HealthSet"));
	//CombatSet = CreateDefaultSubobject<UZodiacCombatSet>(TEXT("CombatSet"));

	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHealthComponent>(this, TEXT("HealthComponent"));

	SlotIndex = INDEX_NONE;
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
		AbilitySystemComponent->DefaultStartingData = HeroData->Attributes;
		HeroName = HeroData->HeroName;
	}

	PlayerCharacter = GetPawnChecked<AZodiacPlayerCharacter>();
	
	Super::OnRegister();
}

void UZodiacHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(LogTemp, Warning, TEXT("default health of %s: %.1f"), *HeroName.ToString(), HealthSet->GetHealth());
	//UE_LOG(LogTemp, Warning, TEXT("default damage of %s: %.1f"), *HeroName.ToString(), CombatSet->GetBaseDamage());	

	ensureMsgf(HeroData, TEXT("Must have HeroData"));
}

UZodiacAbilitySystemComponent* UZodiacHeroComponent::InitializeAbilitySystem()
{
	if (HeroData)
	{
		AddAbilities();
		AbilitySystemComponent->InitAbilityActorInfo(GetOwner(), GetOwner());
		
		HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);

		return AbilitySystemComponent;
	}

	return nullptr;
}

void UZodiacHeroComponent::ActivateHero()
{
	PlayerCharacter->ChangeHeroMesh(HeroData->HeroMesh);
	PlayerCharacter->ChangeCharacterMesh(HeroData->InvisibleMesh, HeroData->HeroAnimInstance);
	
	OnHeroChanged.Broadcast(HeroData->MuzzleSocketNames);
}

void UZodiacHeroComponent::DeactivateHero()
{
}

void UZodiacHeroComponent::AddAbilities()
{
	if (HeroData->AbilitySets.Num() > 0)
	{
		for (TObjectPtr<UZodiacAbilitySet> AbilitySet : HeroData->AbilitySets)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}
}

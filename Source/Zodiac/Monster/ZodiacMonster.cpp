// the.quiet.string@gmail.com


#include "Monster/ZodiacMonster.h"

#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"
#include "Character/ZodiacAttributeManagerComponent.h"


AZodiacMonster::AZodiacMonster(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeManagerComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAttributeManagerComponent>(this, TEXT("AttributeManagerComponent"));
}

void AZodiacMonster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InitializeAbilitySystemComponent();
}

void AZodiacMonster::BeginPlay()
{
	Super::BeginPlay();
}

UZodiacAbilitySystemComponent* AZodiacMonster::GetZodiacAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAbilitySystemComponent* AZodiacMonster::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AZodiacMonster::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetOwnedGameplayTags(TagContainer);
	}
}

bool AZodiacMonster::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool AZodiacMonster::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool AZodiacMonster::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}

void AZodiacMonster::InitializeAbilitySystemComponent()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AddAbilities();

	AttributeManagerComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
}

void AZodiacMonster::AddAbilities()
{
	for (TObjectPtr<UZodiacAbilitySet> AbilitySet : Abilities)
	{
		AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr, nullptr);
	}
}

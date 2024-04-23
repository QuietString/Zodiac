// the.quiet.string@gmail.com


#include "Monster/ZodiacMonster.h"

#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"

AZodiacMonster::AZodiacMonster()
{

}

void AZodiacMonster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AddAbilities();
}

void AZodiacMonster::BeginPlay()
{
	Super::BeginPlay();
	
}

UZodiacAbilitySystemComponent* AZodiacMonster::GetZodiacAbilitySystemComponent() const
{
	return Cast<UZodiacAbilitySystemComponent>(AbilitySystemComponent);
}

UAbilitySystemComponent* AZodiacMonster::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AZodiacMonster::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
}

bool AZodiacMonster::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	return IGameplayTagAssetInterface::HasMatchingGameplayTag(TagToCheck);
}

bool AZodiacMonster::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	return IGameplayTagAssetInterface::HasAllMatchingGameplayTags(TagContainer);
}

bool AZodiacMonster::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	return IGameplayTagAssetInterface::HasAnyMatchingGameplayTags(TagContainer);
}

void AZodiacMonster::InitializeAbilitySystemComponent()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AddAbilities();

	
}

void AZodiacMonster::AddAbilities()
{
	for (TObjectPtr<UZodiacAbilitySet> AbilitySet : Abilities)
	{
		AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
	}
}

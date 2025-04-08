// the.quiet.string@gmail.com


#include "ZodiacAITestCharacter.h"

#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAITestCharacter)

AZodiacAITestCharacter::AZodiacAITestCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, "AbilitySystemComponent");
}

UAbilitySystemComponent* AZodiacAITestCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AZodiacAITestCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetOwnedGameplayTags(TagContainer);
	}
}

bool AZodiacAITestCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool AZodiacAITestCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool AZodiacAITestCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}

void AZodiacAITestCharacter::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->AddLooseGameplayTag(ZodiacGameplayTags::Player_PlayReady);
}

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacHostAbilitySystemComponent.generated.h"

UCLASS()
class ZODIAC_API UZodiacHostAbilitySystemComponent : public UZodiacAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Block and cancel hero abilities too.
	virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;

	// When should call ApplyAbilityBlockAndCancelTags from HeroASC, don't call HeroASC's one. 
	virtual void ApplyAbilityBlockAndCancelTags_FromHeroASC(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags);
};
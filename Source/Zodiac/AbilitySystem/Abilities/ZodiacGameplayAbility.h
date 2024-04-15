// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ZodiacGameplayAbility.generated.h"

/**
 *	The base gameplay ability class used by this project.
 */
UCLASS(Abstract, HideCategories = Input, Meta = (ShortTooltip = "The base gameplay ability class used by this project."))
class ZODIAC_API UZodiacGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> DamageEffect;
};

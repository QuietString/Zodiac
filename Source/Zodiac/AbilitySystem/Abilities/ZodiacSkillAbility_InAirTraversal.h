// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacGameplayAbility.h"
#include "AbilitySystem/Skills/ZodiacSkillAbility.h"
#include "ZodiacSkillAbility_InAirTraversal.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class ZODIAC_API UZodiacSkillAbility_InAirTraversal : public UZodiacSkillAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

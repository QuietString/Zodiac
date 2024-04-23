// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "ZodiacGameplayAbility_ChangeChar.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class ZODIAC_API UZodiacGameplayAbility_ChangeChar : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};

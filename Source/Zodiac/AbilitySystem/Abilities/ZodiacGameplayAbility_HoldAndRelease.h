// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacGameplayAbility.h"
#include "ZodiacGameplayAbility_HoldAndRelease.generated.h"

UCLASS()
class ZODIAC_API UZodiacGameplayAbility_HoldAndRelease : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	UFUNCTION(BlueprintNativeEvent)
	void OnInputRelease(float TimeHeld);
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHostAbility.h"
#include "ZodiacHostAbility_Sprint.generated.h"

class UAbilityTask_WaitInputRelease;

UCLASS(Abstract)
class ZODIAC_API UZodiacHostAbility_Sprint : public UZodiacHostAbility
{
	GENERATED_BODY()

public:
	UZodiacHostAbility_Sprint(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnInputRelease(float TimeHeld);

private:
	UPROPERTY()
	UAbilityTask_WaitInputRelease* WaitInputRelease;
};

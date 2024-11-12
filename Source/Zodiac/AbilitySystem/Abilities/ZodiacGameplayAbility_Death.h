// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacGameplayAbility.h"
#include "ZodiacGameplayAbility_Death.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class ZODIAC_API UZodiacGameplayAbility_Death : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:

	UZodiacGameplayAbility_Death(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	// Starts the death sequence.
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	void StartDeath();

	// Finishes the death sequence.
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	void FinishDeath();

	// If enabled, the ability will automatically call StartDeath.  FinishDeath is always called when the ability ends if the death was started.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Death")
	bool bAutoStartDeath;
};

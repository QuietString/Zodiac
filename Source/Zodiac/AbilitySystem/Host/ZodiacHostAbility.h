// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "ZodiacHostAbility.generated.h"

/**
 * 
 */
UCLASS(abstract)
class ZODIAC_API UZodiacHostAbility : public UZodiacGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData = nullptr) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags|Host", meta = (EditCondition = "bIsHostAbility"))
	FGameplayTagContainer CancelAbilitiesWithTag_Hero;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags|Host", meta = (EditCondition = "bIsHostAbility"))
	FGameplayTagContainer BlockAbilitiesWithTag_Hero;
};

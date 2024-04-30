// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "ZodiacGameplayAbility_Ranged.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacGameplayAbility_Ranged : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Cues")
	FGameplayTag GameplayCueTag_Firing;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Cues")
	FGameplayTag GameplayCueTag_Impact;

private:
	
	UPROPERTY()
	FGameplayCueParameters GCNParameter;
};

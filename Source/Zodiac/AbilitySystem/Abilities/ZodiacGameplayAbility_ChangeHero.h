// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacGameplayAbility.h"
#include "ZodiacGameplayAbility_ChangeHero.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacGameplayAbility_ChangeHero : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:

	UZodiacGameplayAbility_ChangeHero(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	// Slot that this ability controls.
	UPROPERTY(EditDefaultsOnly)
	int32 SlotIndex = INDEX_NONE;
};

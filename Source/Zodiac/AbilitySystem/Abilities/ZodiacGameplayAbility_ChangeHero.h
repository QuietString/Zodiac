// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacGameplayAbility.h"
#include "ZodiacGameplayAbility_ChangeHero.generated.h"


UCLASS()
class ZODIAC_API UZodiacGameplayAbility_ChangeHero : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	// A slot to this ability change to.
	UPROPERTY(EditDefaultsOnly)
	int32 SlotIndex = INDEX_NONE;
};

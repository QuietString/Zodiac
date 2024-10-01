// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHeroAbility.h"
#include "ZodiacHeroAbility_ChangeHero.generated.h"


UCLASS()
class ZODIAC_API UZodiacHeroAbility_ChangeHero : public UZodiacHeroAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	// A slot to this ability change to.
	UPROPERTY(EditDefaultsOnly)
	int32 SlotIndex = INDEX_NONE;
};

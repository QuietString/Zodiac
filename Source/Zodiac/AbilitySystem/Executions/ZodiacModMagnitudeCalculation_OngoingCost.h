// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "ZodiacModMagnitudeCalculation_OngoingCost.generated.h"

UCLASS()
class ZODIAC_API UZodiacModMagnitudeCalculation_OngoingCost : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
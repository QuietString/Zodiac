// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "ZodiacModMagnitudeCalculation_CostAmount.generated.h"


UCLASS()
class ZODIAC_API UZodiacModMagnitudeCalculation_CostAmount : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};

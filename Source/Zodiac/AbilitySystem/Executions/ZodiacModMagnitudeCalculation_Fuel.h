// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "ZodiacModMagnitudeCalculation_Fuel.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacModMagnitudeCalculation_Fuel : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UZodiacModMagnitudeCalculation_Fuel();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

protected:
	FGameplayEffectAttributeCaptureDefinition FuelDef;
	FGameplayEffectAttributeCaptureDefinition InitialCheckAmountDef;
	FGameplayEffectAttributeCaptureDefinition InitialApplyAmountDef;
};

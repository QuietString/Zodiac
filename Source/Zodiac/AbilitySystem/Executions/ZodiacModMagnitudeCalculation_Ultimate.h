// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "ZodiacModMagnitudeCalculation_Ultimate.generated.h"

/**
 * MMC for more conditional dynamic amount;
 */
UCLASS()
class ZODIAC_API UZodiacModMagnitudeCalculation_Ultimate : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UZodiacModMagnitudeCalculation_Ultimate();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

protected:
	FGameplayEffectAttributeCaptureDefinition UltimateDef;
	FGameplayEffectAttributeCaptureDefinition InitialCheckAmountDef;;
	FGameplayEffectAttributeCaptureDefinition InitialApplyAmountDef;
};

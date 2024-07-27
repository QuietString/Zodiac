// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "ZodiacMMC_SkillCost.generated.h"

UCLASS()
class ZODIAC_API UZodiacMMC_SkillCost : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};

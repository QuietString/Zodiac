// the.quiet.string@gmail.com


#include "ZodiacModMagnitudeCalculation_Ultimate.h"

#include "AbilitySystem/Attributes/ZodiacUltimateSet.h"
#include "AbilitySystem/Hero/ZodiacHeroAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacModMagnitudeCalculation_Ultimate)

UZodiacModMagnitudeCalculation_Ultimate::UZodiacModMagnitudeCalculation_Ultimate()
{
	UltimateDef.AttributeToCapture = UZodiacUltimateSet::GetUltimateAttribute();
	UltimateDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	UltimateDef.bSnapshot = false;

	InitialCheckAmountDef.AttributeToCapture = UZodiacUltimateSet::GetInitialCheckAmountAttribute();
	InitialCheckAmountDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	InitialCheckAmountDef.bSnapshot = false;
	
	InitialApplyAmountDef.AttributeToCapture = UZodiacUltimateSet::GetInitialApplyAmountAttribute();
	InitialApplyAmountDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	InitialApplyAmountDef.bSnapshot = false;
	
	RelevantAttributesToCapture.Add(UltimateDef);
	RelevantAttributesToCapture.Add(InitialCheckAmountDef);
	RelevantAttributesToCapture.Add(InitialApplyAmountDef);
}

float UZodiacModMagnitudeCalculation_Ultimate::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const UZodiacHeroAbility* HeroAbility = Cast<UZodiacHeroAbility>(Spec.GetContext().GetAbilityInstance_NotReplicated());
	check(HeroAbility);
	
	// Gather the tags from the source and target as that can affect which buffs should be used
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Ultimate = 0.f;
	GetCapturedAttributeMagnitude(UltimateDef, Spec, EvaluationParameters, Ultimate);
	Ultimate = FMath::Max<float>(Ultimate, 0.0f);

	if (!HeroAbility->GetHasInitialCost())
	{
		return HeroAbility->GetCostAmount();
	}
	else
	{
		float InitialCheckAmount = 0.f;
		GetCapturedAttributeMagnitude(InitialCheckAmountDef, Spec, EvaluationParameters, InitialCheckAmount);
		InitialCheckAmount = FMath::Max<float>(InitialCheckAmount, 1.0f);
	
		float InitialApplyAmount = 0.f;
		GetCapturedAttributeMagnitude(InitialApplyAmountDef, Spec, EvaluationParameters, InitialApplyAmount);
		InitialApplyAmount = FMath::Max<float>(InitialApplyAmount, 1.0f);

		// Current calculation is for initial CheckCost() for an ability with duration.
		if (!HeroAbility->GetHasCheckedInitialCost())
		{
			// Return 0 to make it pass CheckCost() when it exceeds minimum amount. 
			return (Ultimate >= InitialCheckAmount) ? 0.f : FLT_MAX;
		}
		// Current calculation is for ApplyCost()
		else
		{
			if (HeroAbility->ShouldUseInitialCost())
			{
				return InitialApplyAmount;
			}
			else
			{
				return HeroAbility->GetCostAmount();
			}
		}
	}
}
// the.quiet.string@gmail.com

#include "ZodiacModMagnitudeCalculation_OngoingCost.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Skills/ZodiacHeroAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacModMagnitudeCalculation_OngoingCost)

float UZodiacModMagnitudeCalculation_OngoingCost::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	if (const UZodiacHeroAbility* HeroAbility = Cast<UZodiacHeroAbility>(Spec.GetContext().GetAbilityInstance_NotReplicated()))
	{
		return HeroAbility->GetOngoingCostAmount();
	}
	
	return 0.0f;
}

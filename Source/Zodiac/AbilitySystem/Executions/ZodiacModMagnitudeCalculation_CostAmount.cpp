// the.quiet.string@gmail.com


#include "ZodiacModMagnitudeCalculation_CostAmount.h"

#include "AbilitySystem/Skills/ZodiacHeroAbility.h"
#include "Character/ZodiacHeroCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacModMagnitudeCalculation_CostAmount)

float UZodiacModMagnitudeCalculation_CostAmount::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	if (const UZodiacHeroAbility* HeroAbility = Cast<UZodiacHeroAbility>(Spec.GetContext().GetAbilityInstance_NotReplicated()))
	{
		return HeroAbility->GetCostToApply();
	}
	
	return 0.0f;
}

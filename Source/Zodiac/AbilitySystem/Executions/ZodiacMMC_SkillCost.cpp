// the.quiet.string@gmail.com


#include "ZodiacMMC_SkillCost.h"

#include "AbilitySystem/Skills/ZodiacSkillAbility.h"

float UZodiacMMC_SkillCost::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	if (const UZodiacSkillAbility* SkillAbility = Cast<UZodiacSkillAbility>(Spec.GetContext().GetAbility()))
	{
		return SkillAbility->GetRequiredCostAmount();
	}
	
	return 0.0f;
}

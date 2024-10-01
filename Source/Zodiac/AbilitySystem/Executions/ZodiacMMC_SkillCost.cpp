// the.quiet.string@gmail.com

#include "ZodiacMMC_SkillCost.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Skills/ZodiacHeroAbility.h"

float UZodiacMMC_SkillCost::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// if (const UZodiacSkillAbility* SkillAbility = Cast<UZodiacSkillAbility>(Spec.GetContext().GetAbilityInstance_NotReplicated()))
	// {
	// 	return SkillAbility->GetCurrentGECostAmount();
	// }
	
	return 0.0f;
}

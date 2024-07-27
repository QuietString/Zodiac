// the.quiet.string@gmail.com

#include "ZodiacSkillAbilityCost_TagStack.h"

#include "NativeGameplayTags.h"
#include "AbilitySystem/Skills/ZodiacSkillSlot.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacSkillAbilityCost_TagStack)

UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_FAIL_COST, "Ability.ActivateFail.Cost");

UZodiacSkillAbilityCost_TagStack::UZodiacSkillAbilityCost_TagStack()
{
	Quantity.SetValue(1.0f);
	FailureTag = TAG_ABILITY_FAIL_COST;
}

bool UZodiacSkillAbilityCost_TagStack::CheckCost(const UZodiacSkillAbility* SkillAbility,
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (SkillAbility)
	{
		const float NumStacksReal = Quantity.GetValueAtLevel(1);
		const int32 NumStacks = FMath::TruncToInt(NumStacksReal);
		
		int32 NumStacksFound = 0;
		if (const UZodiacSkillAbility* Skill = Cast<UZodiacSkillAbility>(SkillAbility))
		{
			if (const UZodiacSkillSlot* SkillSlot = Skill->GetSkillSlot())
			{
				NumStacksFound = SkillSlot->GetStatTagStackCount(Tag);		
			}
		}
		
		const bool bCanApplyCost = NumStacksFound >= NumStacks;
		
		// Inform other abilities why this cost cannot be applied
		if (!bCanApplyCost && OptionalRelevantTags && FailureTag.IsValid())
		{
			OptionalRelevantTags->AddTag(FailureTag);				
		}
		return bCanApplyCost;
	}
	
	return false;
}

void UZodiacSkillAbilityCost_TagStack::ApplyCost(const UZodiacSkillAbility* SkillAbility,
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority() && SkillAbility)
	{
		const float NumStacksReal = Quantity.GetValueAtLevel(1);
		const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

		if (const UZodiacSkillAbility* Skill = Cast<UZodiacSkillAbility>(SkillAbility))
		{
			if (UZodiacSkillSlot* SkillSlot = Skill->GetSkillSlot())
			{
				SkillSlot->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}

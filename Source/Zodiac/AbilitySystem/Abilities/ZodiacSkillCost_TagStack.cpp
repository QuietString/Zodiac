// the.quiet.string@gmail.com

#include "ZodiacSkillCost_TagStack.h"

#include "NativeGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Skills/ZodiacHeroAbility.h"
#include "AbilitySystem/Skills/ZodiacSkillSlot.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacSkillCost_TagStack)

//UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_FAIL_COST, "Ability.ActivateFail.Cost");

UZodiacSkillCost_TagStack::UZodiacSkillCost_TagStack()
{
	Quantity.SetValue(1.0f);
	//FailureTag = TAG_ABILITY_FAIL_COST;
}

bool UZodiacSkillCost_TagStack::CheckCost(const UZodiacGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Ability)
	{
		const float NumStacksReal = Quantity.GetValueAtLevel(1);
		const int32 NumStacks = FMath::TruncToInt(NumStacksReal);
		
		int32 NumStacksFound = 0;
		if (const UZodiacHeroAbility* Skill = Cast<UZodiacHeroAbility>(Ability))
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

void UZodiacSkillCost_TagStack::ApplyCost(const UZodiacGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority() && Ability)
	{
		const float NumStacksReal = Quantity.GetValueAtLevel(1);
		const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

		if (const UZodiacHeroAbility* Skill = Cast<UZodiacHeroAbility>(Ability))
		{
			if (UZodiacSkillSlot* SkillSlot = Skill->GetSkillSlot())
			{
				SkillSlot->RemoveStatTagStack(Tag, NumStacks);		
			}
		}
	}
}
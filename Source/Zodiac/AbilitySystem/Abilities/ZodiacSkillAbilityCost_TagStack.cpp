// the.quiet.string@gmail.com

#include "ZodiacSkillAbilityCost_TagStack.h"

#include "NativeGameplayTags.h"
#include "Item/ZodiacHeroItemSlot.h"
#include "Item/ZodiacSkillInstance.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacSkillAbilityCost_TagStack)

UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_FAIL_COST, "Ability.ActivateFail.Cost");

UZodiacSkillAbilityCost_TagStack::UZodiacSkillAbilityCost_TagStack()
{
	Quantity.SetValue(1.0f);
	FailureTag = TAG_ABILITY_FAIL_COST;
}

bool UZodiacSkillAbilityCost_TagStack::CheckCost(const UZodiacHeroAbility* SkillAbility,
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (SkillAbility)
	{
		const float NumStacksReal = Quantity.GetValueAtLevel(1);
		const int32 NumStacks = FMath::TruncToInt(NumStacksReal);
		
		int32 NumStacksFound = 0;
		if (const UZodiacHeroAbility* Skill = Cast<UZodiacHeroAbility>(SkillAbility))
		{
			if (const UZodiacHeroItemSlot* ItemSlot = Skill->GetAssociatedSlot())
			{
				NumStacksFound = ItemSlot->GetStatTagStackCount(Tag);		
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

void UZodiacSkillAbilityCost_TagStack::ApplyCost(const UZodiacHeroAbility* SkillAbility,
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority() && SkillAbility)
	{
		const float NumStacksReal = Quantity.GetValueAtLevel(1);
		const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

		if (const UZodiacHeroAbility* Skill = Cast<UZodiacHeroAbility>(SkillAbility))
		{
			if (UZodiacHeroItemSlot* ItemSlot = Skill->GetAssociatedSlot())
			{
				ItemSlot->RemoveStatTagStack(Tag, NumStacks);
			}
		}
	}
}

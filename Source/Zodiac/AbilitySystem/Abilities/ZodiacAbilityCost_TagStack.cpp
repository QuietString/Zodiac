// the.quiet.string@gmail.com

#include "ZodiacAbilityCost_TagStack.h"

#include "NativeGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Skills/ZodiacHeroAbility.h"
#include "Item/ZodiacHeroItemSlot.h"
#include "Item/ZodiacSkillInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAbilityCost_TagStack)

//UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_FAIL_COST, "Ability.ActivateFail.Cost");

UZodiacAbilityCost_TagStack::UZodiacAbilityCost_TagStack()
{
	Quantity.SetValue(1.0f);
	//FailureTag = TAG_ABILITY_FAIL_COST;
}

bool UZodiacAbilityCost_TagStack::CheckCost(const UZodiacGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Ability)
	{
		const float NumStacksReal = Quantity.GetValueAtLevel(1);
		const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

		int32 NumStacksFound = 0;
		if (const UZodiacHeroAbility* Skill = Cast<UZodiacHeroAbility>(Ability))
		{
			if (UZodiacHeroItemSlot* ItemSlot = Skill->GetAssociatedSlot())
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

void UZodiacAbilityCost_TagStack::ApplyCost(const UZodiacGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority() && Ability)
	{
		const float NumStacksReal = Quantity.GetValueAtLevel(1);
		const int32 NumStacks = FMath::TruncToInt(NumStacksReal);

		if (const UZodiacHeroAbility* Skill = Cast<UZodiacHeroAbility>(Ability))
		{
			if (UZodiacHeroItemSlot* SkillSlot = Skill->GetAssociatedSlot())
			{
				SkillSlot->RemoveStatTagStack(Tag, NumStacks);		
			}
		}
	}
}
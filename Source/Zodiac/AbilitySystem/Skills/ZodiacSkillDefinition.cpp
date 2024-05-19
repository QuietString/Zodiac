// the.quiet.string@gmail.com


#include "ZodiacSkillDefinition.h"

#include "AbilitySystem/ZodiacAbilitySystemComponent.h"

void UZodiacSkillDefinition::GiveToAbilitySystemComponent(UZodiacAbilitySystemComponent* ZodiacASC)
{
	check(ZodiacASC);

	if (!ZodiacASC->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	UZodiacGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UZodiacGameplayAbility>();
	FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
	AbilitySpec.DynamicAbilityTags.AddTag(AbilityToGrant.InputTag);
		
	const FGameplayAbilitySpecHandle AbilitySpecHandle = ZodiacASC->GiveAbility(AbilitySpec);

	FSkillHandleData Data;
	Data.SkillID = SkillID;
	Data.SlotType = SlotType;
	Data.Handle = AbilitySpecHandle;
	ZodiacASC->SkillHandles.AddSkillHandle(Data);
}

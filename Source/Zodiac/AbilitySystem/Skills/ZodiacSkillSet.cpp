// the.quiet.string@gmail.com


#include "ZodiacSkillSet.h"

#include "GameplayAbilitySpec.h"
#include "ZodiacLogChannels.h"
#include "ZodiacSkillAbility.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"

UZodiacSkillSet::UZodiacSkillSet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Type = EZodiacAbilitySetType::Skill;
}

void UZodiacSkillSet::GiveToAbilitySystem(UZodiacAbilitySystemComponent* ZodiacASC,
                                          FZodiacAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	Super::GiveToAbilitySystem(ZodiacASC, OutGrantedHandles, SourceObject);

	// Grant the skill abilities.
	for (int32 SkillIndex = 0; SkillIndex < GrantedSkillAbilities.Num(); ++SkillIndex)
	{
		const FZodiacSkillSet_GrantedSkillAbility& SkillToGrant = GrantedSkillAbilities[SkillIndex];

		if (!IsValid(SkillToGrant.Ability))
		{
			UE_LOG(LogZodiacAbilitySystem, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s] is not valid."), SkillIndex, *GetNameSafe(this));
			continue;
		}

		UZodiacSkillAbility* AbilityCDO = SkillToGrant.Ability->GetDefaultObject<UZodiacSkillAbility>();
		FGameplayAbilitySpec AbilitySpec(AbilityCDO, SkillToGrant.AbilityLevel, INDEX_NONE, SourceObject);
		AbilitySpec.DynamicAbilityTags.AddTag(SkillToGrant.InputTag);
		
		const FGameplayAbilitySpecHandle AbilitySpecHandle = ZodiacASC->GiveAbility(AbilitySpec);
		
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}
}
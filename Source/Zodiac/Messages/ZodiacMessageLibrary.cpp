// the.quiet.string@gmail.com

#include "ZodiacMessageLibrary.h"

#include "ZodiacGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ABILITY_TYPE_SKILL_PRIMARY, "Ability.Type.Skill.Primary");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ABILITY_TYPE_SKILL_SECONDARY, "Ability.Type.Skill.Secondary");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ABILITY_TYPE_SKILL_ULTIMATE, "Ability.Type.Skill.Ultimate");

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ABILITY_TYPE_SKILL_PRIMARY_COOLDOWN_MESSAGE, "Ability.Type.Skill.Primary.Cooldown.Message");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ABILITY_TYPE_SKILL_SECONDARY_COOLDOWN_MESSAGE, "Ability.Type.Skill.Secondary.Cooldown.Message");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ABILITY_TYPE_SKILL_ULTIMATE_COOLDOWN_MESSAGE, "Ability.Type.Skill.Ultimate.Cooldown.Message");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ABILITY_TYPE_SKILL_ULTIMATE_CHARGE_MESSAGE, "Ability.Type.Skill.Ultimate.Charge.Message");

FGameplayTag UZodiacMessageLibrary::GetCooldownChannelByTag(const FGameplayTag Tag)
{
	FGameplayTag MessageChannel;
	
	if (Tag.MatchesTag(TAG_ABILITY_TYPE_SKILL_PRIMARY))
	{
		MessageChannel = TAG_ABILITY_TYPE_SKILL_PRIMARY_COOLDOWN_MESSAGE;
	}
	else if (Tag.MatchesTag(TAG_ABILITY_TYPE_SKILL_SECONDARY))
	{
		MessageChannel = TAG_ABILITY_TYPE_SKILL_SECONDARY_COOLDOWN_MESSAGE;
	}
	else if (Tag.MatchesTag(TAG_ABILITY_TYPE_SKILL_ULTIMATE))
	{
		MessageChannel = TAG_ABILITY_TYPE_SKILL_ULTIMATE_COOLDOWN_MESSAGE;
	}

	return  MessageChannel;
}

FGameplayTag UZodiacMessageLibrary::GetCooldownChannelByTags(const FGameplayTagContainer& TagContainer)
{
	FGameplayTag MessageChannel;

	if (TagContainer.HasTag(TAG_ABILITY_TYPE_SKILL_PRIMARY))
	{
		MessageChannel = TAG_ABILITY_TYPE_SKILL_PRIMARY_COOLDOWN_MESSAGE;
	}
	else if (TagContainer.HasTag(TAG_ABILITY_TYPE_SKILL_SECONDARY))
	{
		MessageChannel = TAG_ABILITY_TYPE_SKILL_SECONDARY_COOLDOWN_MESSAGE;
	}
	else if (TagContainer.HasTag(TAG_ABILITY_TYPE_SKILL_ULTIMATE))
	{
		MessageChannel = TAG_ABILITY_TYPE_SKILL_ULTIMATE_COOLDOWN_MESSAGE;
	}

	return  MessageChannel;
}

FGameplayTag UZodiacMessageLibrary::GetUltimateChargeChannel()
{
	return TAG_ABILITY_TYPE_SKILL_ULTIMATE_CHARGE_MESSAGE;
}

FGameplayTag UZodiacMessageLibrary::GetSkillChangeChannelByTag(const FGameplayTag Tag)
{
	FGameplayTag MessageChannel;
	
	if (Tag.MatchesTag(ZodiacGameplayTags::Ability_Type_Skill_Primary))
	{
		MessageChannel = ZodiacGameplayTags::Ability_HeroChanged_PrimarySkill_Message;
	}
	else if (Tag.MatchesTag(ZodiacGameplayTags::Ability_Type_Skill_Secondary))
	{
		MessageChannel = ZodiacGameplayTags::Ability_HeroChanged_SecondarySkill_Message;
	}
	else if (Tag.MatchesTag(ZodiacGameplayTags::Ability_Type_Skill_Ultimate))
	{
		MessageChannel = ZodiacGameplayTags::Ability_HeroChanged_UltimateSkill_Message;
	}

	return  MessageChannel;
}

FGameplayTag UZodiacMessageLibrary::GetSkillChangeChannelByTags(const FGameplayTagContainer& TagContainer)
{
	FGameplayTag MessageChannel;

	if (TagContainer.HasTag(TAG_ABILITY_TYPE_SKILL_PRIMARY))
	{
		MessageChannel = ZodiacGameplayTags::Ability_HeroChanged_PrimarySkill_Message;
	}
	else if (TagContainer.HasTag(TAG_ABILITY_TYPE_SKILL_SECONDARY))
	{
		MessageChannel = ZodiacGameplayTags::Ability_HeroChanged_SecondarySkill_Message;
	}
	else if (TagContainer.HasTag(TAG_ABILITY_TYPE_SKILL_ULTIMATE))
	{
		MessageChannel = ZodiacGameplayTags::Ability_HeroChanged_UltimateSkill_Message;
	}

	return  MessageChannel;
}

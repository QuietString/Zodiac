// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacGameplayTags.h"

#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"
#include "ZodiacLogChannels.h"

namespace ZodiacGameplayTags
{
	// Heroes
	UE_DEFINE_GAMEPLAY_TAG(Hero_Name_Twinblast, "Hero.Name.Twinblast");
	UE_DEFINE_GAMEPLAY_TAG(Hero_Name_Murdock, "Hero.Name.Murdock");
	UE_DEFINE_GAMEPLAY_TAG(Hero_Slot_Slot1, "Hero.Slot.Slot1");
	UE_DEFINE_GAMEPLAY_TAG(Hero_Slot_Slot2, "Hero.Slot.Slot2");
	UE_DEFINE_GAMEPLAY_TAG(Hero_Changed_HealthBar_Message, "Hero.Changed.HealthBar.Message");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Networking, "Ability.ActivateFail.Networking", "Ability failed to activate because it did not pass the network checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Action_Jump, "Ability.Type.Action.Jump");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Skill_Slot_Primary, "Ability.Type.Skill.Slot.Primary");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Skill_Slot_Secondary, "Ability.Type.Skill.Slot.Secondary");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Type_Skill_Slot_Ultimate, "Ability.Type.Skill.Slot.Ultimate");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Type_Skill_Cost_None, "Ability.Type.Skill.Cost.None", "A skill that can be used without any cost");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Type_Skill_Cost_Bullet, "Ability.Type.Skill.Cost.Bullet", "A skill that need to consume some number of items");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Type_Skill_Cost_Cooldown, "Ability.Type.Skill.Cost.Cooldown", "A skill that has to wait certain time after activation");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Type_Skill_Cost_Gauge, "Ability.Type.Skill.Cost.Gauge", "A skill that need fully charged gauage to activate");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Type_Skill_Cost_Ultimate, "Ability.Type.Skill.Cost.Ultimate", "A skill that need hero's ultimate gauage to activate");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Cost_Stack, "Ability.Cost.Stack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Cost_Cooldown, "Ability.Cost.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Cost_Attribute, "Ability.Cost.Attribute");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Stack, "Ability.Stack");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath", "An ability with this type tag should not be canceled due to death.");

	UE_DEFINE_GAMEPLAY_TAG(Ability_HeroChanged_PrimarySkill_Message , "Ability.HeroChanged.PrimarySkill.Message")
	UE_DEFINE_GAMEPLAY_TAG(Ability_HeroChanged_SecondarySkill_Message , "Ability.HeroChanged.SecondarySkill.Message")
	UE_DEFINE_GAMEPLAY_TAG(Ability_HeroChanged_UltimateSkill_Message , "Ability.HeroChanged.Ultimate.Message")
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move_Sprint, "InputTag.Move.Sprint", "Sprint direction input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Look (mouse) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Stick, "InputTag.Look.Stick", "Look (stick) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Crouch, "InputTag.Crouch", "Crouch input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_AutoRun, "InputTag.AutoRun", "Auto-run input.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded/replicated and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");

	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Damaged_Message, "GameplayEvent.Damaged.Message");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Death, "GameplayEvent.Death", "Event that fires on death. This event only fires on the server.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Reset, "GameplayEvent.Reset", "Event that fires once a player reset is executed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_RequestReset, "GameplayEvent.RequestReset", "Event to request a player's pawn to be instantly replaced with a new one at a valid spawn location.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Damage, "SetByCaller.Damage", "SetByCaller tag used by damage gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_SkillMultiplier, "SetByCaller.SkillMultiplier", "SetByCaller tag used to apply skill multiplier.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Heal, "SetByCaller.Heal", "SetByCaller tag used by healing gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Cooldown, "SetByCaller.Cooldown", "SetByCaller tag used to apply cooldown gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_UltimateGauge, "SetByCaller.Ultimate.Charge", "SetByCaller tag used to apply charge ultimate gauge gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Ultimate_Cost, "SetByCaller.Ultimate.Cost");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_GodMode, "Cheat.GodMode", "GodMode cheat is active on the owner.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_UnlimitedHealth, "Cheat.UnlimitedHealth", "UnlimitedHealth cheat is active on the owner.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Crouching, "Status.Crouching", "Target is crouching.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_AutoRunning, "Status.AutoRunning", "Target is auto-running.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death, "Status.Death", "Target has the death status.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dying, "Status.Death.Dying", "Target has begun the death process.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dead, "Status.Death.Dead", "Target has finished the death process.");
	UE_DEFINE_GAMEPLAY_TAG(Status_Stun, "Status.Stun");

	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Menu, "UI.Layer.Menu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Game, "UI.Layer.Game");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Modal, "UI.Layer.Modal");
	UE_DEFINE_GAMEPLAY_TAG(UI_Action_Escape, "UI.Action.Escape");
	
	UE_DEFINE_GAMEPLAY_TAG(HUD_Message_HeroChanged_SkillSlot, "HUD.Message.HeroChanged.SkillSlot");
	UE_DEFINE_GAMEPLAY_TAG(HUD_Message_HeroChanged, "HUD.Message.HeroChanged");

	UE_DEFINE_GAMEPLAY_TAG(HUD_Message_SkillDuration, "HUD.Message.SkillDuration");
	UE_DEFINE_GAMEPLAY_TAG(HUD_Message_SkillCommit, "HUD.Message.SkillCommit");
	UE_DEFINE_GAMEPLAY_TAG(HUD_Message_SkillSlot_TagStackChanged, "HUD.Message.SkillSlot.TagStackChanged");
	UE_DEFINE_GAMEPLAY_TAG(HUD_Message_SkillSlot_Created, "HUD.Message.SkillSlot.Created");
	UE_DEFINE_GAMEPLAY_TAG(HUD_Message_AttributeChanged_Ultimate, "HUD.Message.AttributeChanged.Ultimate");
	
	// Tags for GameplayCue
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Rifle_Fire, "GameplayCue.Weapon.Rifle.Fire");

	// These are mapped to the movement modes inside GetMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Walking, "Movement.Mode.Walking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_NavWalking, "Movement.Mode.NavWalking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Falling, "Movement.Mode.Falling", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Swimming, "Movement.Mode.Swimming", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Flying, "Movement.Mode.Flying", "Default Character movement tag");

	// When extending Zodiac, you can create your own movement modes but you need to update GetCustomMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Custom, "Movement.Mode.Custom", "This is invalid and should be replaced with custom tags.  See ZodiacGameplayTags::CustomMovementModeTagMap.");

	// Unreal Movement Modes
	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking, Movement_Mode_Walking },
		{ MOVE_NavWalking, Movement_Mode_NavWalking },
		{ MOVE_Falling, Movement_Mode_Falling },
		{ MOVE_Swimming, Movement_Mode_Swimming },
		{ MOVE_Flying, Movement_Mode_Flying },
		{ MOVE_Custom, Movement_Mode_Custom }
	};

	// Custom Movement Modes
	const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
	{
		// Fill these in with your custom modes
	};

	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					UE_LOG(LogZodiac, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}

	FGameplayTag GetCooldownExtendedTag(const FGameplayTag& SkillTag)
	{
		FString CooldownTagString = SkillTag.ToString() + TEXT(".") + TEXT("Cooldown");
		FGameplayTag FoundTag = FGameplayTag::RequestGameplayTag(*CooldownTagString);
		
		return FoundTag.IsValid() ? FoundTag : FGameplayTag(); 
	}
}


// the.quiet.string@gmail.com

#include "ZodiacGameplayTags.h"

#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"
#include "Character/ZodiacCharacterType.h"
#include "ZodiacLogChannels.h"

namespace ZodiacGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Networking, "Ability.ActivateFail.Networking", "Ability failed to activate because it did not pass the network checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");
	UE_DEFINE_GAMEPLAY_TAG(Ability_ActivateFail_OutOfAllowedAimRange, "Ability.ActivateFail.OutOfAllowedAimRange");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Cost_Stack, "Ability.Cost.Stack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Cost_Stack_MagazineAmmo, "Ability.Cost.Stack.MagazineAmmo");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Cost_Stack_MagazineSize, "Ability.Cost.Stack.MagazineSize");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Cost_Cooldown, "Ability.Cost.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Cost_Attribute, "Ability.Cost.Attribute");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath", "An ability with this type tag should not be canceled due to death.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move_Sprint, "InputTag.Move.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move_Fly, "InputTag.Move.Fly");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Look (mouse) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Stick, "InputTag.Look.Stick", "Look (stick) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Hero, "InputTag.Hero", "Hero ability input.");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Ability_Traversal, "Event.Ability.Traversal");

	UE_DEFINE_GAMEPLAY_TAG(Event_Damaged_Message, "Event.Damaged.Message");
	UE_DEFINE_GAMEPLAY_TAG(Event_Elimination, "Event.Elimination");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Death, "Event.Death", "Event that fires on death. This event only fires on the server.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Reset, "Event.Reset", "Event that fires once a player reset is executed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_RequestReset, "Event.RequestReset", "Event to request a player's pawn to be instantly replaced with a new one at a valid spawn location.");

	// Effects
	UE_DEFINE_GAMEPLAY_TAG(Effect_Type_Damage, "Effect.Type.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Type_Damage_None, "Effect.Type.Damage.None");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Type_Damage_Bullet, "Effect.Type.Damage.Bullet");
	UE_DEFINE_GAMEPLAY_TAG(Effect_Type_Damage_Explosive, "Effect.Type.Damage.Explosive");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Damage, "SetByCaller.Damage", "SetByCaller tag used by damage gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Duration, "SetByCaller.Duration");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Amount, "SetByCaller.Amount");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Cost, "SetByCaller.Cost");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Heal, "SetByCaller.Heal", "SetByCaller tag used by healing gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Cooldown, "SetByCaller.Cooldown", "SetByCaller tag used to apply cooldown gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Ultimate, "SetByCaller.Ultimate", "SetByCaller tag used to change ultimate attribute value.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Cost_MinimumCheck, "SetByCaller.Cost.MinimumCheck", "SetByCaller tag used to check if the owner has minimum amount of attributes to activate an ability.");
	
	// Cheats
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_GodMode, "Cheat.GodMode", "GodMode cheat is active on the owner.");
	UE_DEFINE_GAMEPLAY_TAG(Cheat_InfiniteAmmo, "Cheat.InfiniteAmmo")
	UE_DEFINE_GAMEPLAY_TAG(Cheat_InfiniteUltimate, "Cheat.InfiniteUltimate")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_UnlimitedHealth, "Cheat.UnlimitedHealth", "UnlimitedHealth cheat is active on the owner.");
	
	UE_DEFINE_GAMEPLAY_TAG(Status_Movement_Attacking, "Status.Movement.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(Status_Movement_Disabled, "Status.Movement.Disabled");
	UE_DEFINE_GAMEPLAY_TAG(Status_Weapon_Firing, "Status.Weapon.Firing");
	UE_DEFINE_GAMEPLAY_TAG(Status_WeaponReady , "Status.WeaponReady");
	UE_DEFINE_GAMEPLAY_TAG(Status_ADS, "Status.ADS");
	UE_DEFINE_GAMEPLAY_TAG(Status_Focus , "Status.Focus");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_MovementAutoRunning, "Status.Movement.AutoRunning", "Target is auto-running.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death, "Status.Death", "Target has the death status.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dying, "Status.Death.Dying", "Target has begun the death process.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dead, "Status.Death.Dead", "Target has finished the death process.");

	UE_DEFINE_GAMEPLAY_TAG(Status_Stun, "Status.Stun");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Invincible, "Status.Invincible", "Target doesn't take any damage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Immortal, "Status.Immortal", "Target's health can't go below 1");
	UE_DEFINE_GAMEPLAY_TAG(Status_Physics_Collision_Disabled, "Status.Physics.Collision.Disabled");

	UE_DEFINE_GAMEPLAY_TAG(Player_PlayReady, "Player.PlayReady")

	UE_DEFINE_GAMEPLAY_TAG(Camera_Aiming_Steady, "Camera.Aiming.Steady");
	UE_DEFINE_GAMEPLAY_TAG(Camera_Aiming_Unstable, "Camera.Aiming.Unstable");
	
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Menu, "UI.Layer.Menu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Game, "UI.Layer.Game");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_GameMenu, "UI.Layer.GameMenu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Modal, "UI.Layer.Modal");
	UE_DEFINE_GAMEPLAY_TAG(UI_Action_Escape, "UI.Action.Escape");

	UE_DEFINE_GAMEPLAY_TAG(HUD_Message_ReticleChanged, "HUD.Message.ReticleChanged");
	UE_DEFINE_GAMEPLAY_TAG(HUD_Message_GlobalReticleChanged, "HUD.Message.GlobalReticleChanged");
	UE_DEFINE_GAMEPLAY_TAG(HUD_Message_HealthChanged, "HUD.Message.HealthChanged");
	UE_DEFINE_GAMEPLAY_TAG(HUD_Message_WidgetChanged, "HUD.Message.WidgetChanged");

	UE_DEFINE_GAMEPLAY_TAG(Message_Gameplay_Elimination, "Message.Gameplay.Elimination");

	// Traversal Actions
	UE_DEFINE_GAMEPLAY_TAG(Traversal_FailReason_NoTraceHit, "Traversal.FailReason.NoTraceHit");
	UE_DEFINE_GAMEPLAY_TAG(Traversal_FailReason_NoFrontLedgeFound, "Traversal.FailReason.NoFrontLedgeFound");
	UE_DEFINE_GAMEPLAY_TAG(Traversal_FailReason_OutOfAngle, "Traversal.FailReason.OutOfAngle");
	UE_DEFINE_GAMEPLAY_TAG(Traversal_FailReason_OutOfDistance, "Traversal.FailReason.OutOfDistance");
	UE_DEFINE_GAMEPLAY_TAG(Traversal_FailReason_BlockedByCeiling, "Traversal.FailReason.BlockedByCeiling");
	UE_DEFINE_GAMEPLAY_TAG(Traversal_FailReason_NoProperActionType, "Traversal.FailReason.NoProperActionType");
	UE_DEFINE_GAMEPLAY_TAG(Traversal_FailReason_NoMatchingMontage, "Traversal.FailReason.NoMatchingMontage");

	
	// These are mapped to the movement modes inside GetMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG(Movement_Mode, "Movement.Mode");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_None, "Movement.Mode.None", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Walking, "Movement.Mode.Walking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_NavWalking, "Movement.Mode.NavWalking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Falling, "Movement.Mode.Falling", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Swimming, "Movement.Mode.Swimming", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Flying, "Movement.Mode.Flying", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Custom, "Movement.Mode.Custom", "Default Character movement tag");

	// Default Movement Modes
	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking, Movement_Mode_Walking },
		{ MOVE_NavWalking, Movement_Mode_NavWalking },
		{ MOVE_Falling, Movement_Mode_Falling },
		{ MOVE_Swimming, Movement_Mode_Swimming },
		{ MOVE_Flying, Movement_Mode_Flying },
		{MOVE_Custom, Movement_Mode_Custom}
	};
	
	UE_DEFINE_GAMEPLAY_TAG(Movement_Extended_None, "Movement.Extended.None");
	UE_DEFINE_GAMEPLAY_TAG(Movement_Extended_Walking, "Movement.Extended.Walking");
	UE_DEFINE_GAMEPLAY_TAG(Movement_Extended_Running, "Movement.Extended.Running");
	UE_DEFINE_GAMEPLAY_TAG(Movement_Extended_Sprinting, "Movement.Extended.Sprinting");

	// Extended Movement Modes
	const TMap<EZodiacExtendedMovementMode, FGameplayTag> ExtendedMovementModeTagMap =
	{
		{ EZodiacExtendedMovementMode::None, Movement_Extended_None },
		{ EZodiacExtendedMovementMode::Walking, Movement_Extended_Walking },
		{ EZodiacExtendedMovementMode::Running, Movement_Extended_Running },
		{ EZodiacExtendedMovementMode::Sprinting, Movement_Extended_Sprinting },
	};
	
	// These are mapped to the movement modes inside GetCustomMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG(Movement_Custom_None, "Movement.Custom.None");
	UE_DEFINE_GAMEPLAY_TAG(Movement_Custom_Traversal, "Movement.Custom.Traversal");
	
	// Custom Movement Modes
	const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
	{
		{Move_Custom_None, Movement_Custom_None},
		{Move_Custom_Traversal, Movement_Custom_Traversal}
	};

	const TMap<FGameplayTag, uint8> TagCustomMovementModeMap =
	{
		{Movement_Custom_None, Move_Custom_None},
		{Movement_Custom_Traversal, Move_Custom_Traversal}
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
}


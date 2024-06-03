// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ESkillSlotType : uint8
{
	None,
	Primary,
	Secondary,
	Ultimate,
};

UENUM(BlueprintType)
enum class ESkillCostType : uint8
{
	None,
	Stack,
	UltimateGauge
	// Cooldown is managed separately
};

/**
 * 
 */

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"

#include "ZodiacSkillTypes.generated.h"

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
 * Sockets used for skill ability FX.
 */
USTRUCT(BlueprintType)
struct FZodiacSkillComboSocketData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FName> ComboSocket;

	UPROPERTY(VisibleAnywhere)
	int32 ComboIndex = 0;
};
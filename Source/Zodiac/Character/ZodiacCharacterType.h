// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacCharacterType.generated.h"

/**
 * Secondary movement modes to distinguish different movement under equal standard MovementMode 
 */
UENUM(BlueprintType, DisplayName = "Extended Movement Mode")
enum class EZodiacExtendedMovementMode : uint8
{
	None,
	Walking,
	Running
};

USTRUCT(BlueprintType, DisplayName = "Extended Movement Config")
struct FZodiacExtendedMovementConfig
{
	GENERATED_BODY()

public:
	// Only Walking and Running mode supports speed.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EZodiacExtendedMovementMode, FVector> MovementSpeedsMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EZodiacExtendedMovementMode DefaultExtendedMovement  = EZodiacExtendedMovementMode::Running;
};
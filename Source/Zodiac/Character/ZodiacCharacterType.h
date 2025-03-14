// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacCharacterType.generated.h"

UENUM(BlueprintType)
enum EZodiacCustomMovementMode
{
	Move_Custom_None = 0			UMETA(DisplayName="None"),
	Move_Custom_Traversal			UMETA(DisplayName="Traversal")
};

/**
 * Secondary movement modes to distinguish different movement under equal standard MovementMode 
 */
UENUM(BlueprintType, DisplayName = "Extended Movement Mode")
enum class EZodiacExtendedMovementMode : uint8
{
	None,
	Walking,
	Running,
	Sprinting
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

USTRUCT()
struct FZodiacZombieSpawnConfig
{
	GENERATED_BODY()

public:
	FZodiacZombieSpawnConfig()
		: MovementConfigTemplateIndex(INDEX_NONE)
		, DefaultMovementMode(EZodiacExtendedMovementMode::None)
		, Seed(INDEX_NONE)
	{}

	FZodiacZombieSpawnConfig(const int32 Index, const EZodiacExtendedMovementMode MovementMode, const int32 Seed)
		: MovementConfigTemplateIndex(Index)
		, DefaultMovementMode(MovementMode)
		, Seed(Seed)
	{}
	
	UPROPERTY()
	int32 MovementConfigTemplateIndex;

	UPROPERTY()
	EZodiacExtendedMovementMode DefaultMovementMode;
	
	UPROPERTY()
	int32 Seed;

	bool IsValid() const { return (MovementConfigTemplateIndex != INDEX_NONE); }
};
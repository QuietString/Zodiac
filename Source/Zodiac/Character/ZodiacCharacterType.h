// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacCharacterType.generated.h"

class UBehaviorTree;

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
		, BehaviorTree(nullptr)
		, bUseTargetSearchRadius(false)
		, TargetSearchRadius(0.f)
		, bRespawnWhenDies(false)
	{}

	FZodiacZombieSpawnConfig(const int32 Index, const EZodiacExtendedMovementMode MovementMode, const int32 Seed, UBehaviorTree* BehaviorTree, const bool bUseTargetSearchRadius, const float 
	TargetSearchRadius, const bool bRespawnWhenDies = false)
		: MovementConfigTemplateIndex(Index)
		, DefaultMovementMode(MovementMode)
		, Seed(Seed)
		, BehaviorTree(BehaviorTree)
		, bUseTargetSearchRadius(bUseTargetSearchRadius)
		, TargetSearchRadius(TargetSearchRadius)
		, bRespawnWhenDies(bRespawnWhenDies)
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

	UPROPERTY(NotReplicated)
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(notreplicated)
	bool bUseTargetSearchRadius;

	UPROPERTY(NotReplicated)
	float TargetSearchRadius;

	UPROPERTY(NotReplicated)
	bool bRespawnWhenDies;
	
	bool IsValid() const { return (MovementConfigTemplateIndex != INDEX_NONE); }
};
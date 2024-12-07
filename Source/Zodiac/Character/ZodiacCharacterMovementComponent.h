// the.quiet.string@gmail.com

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "NativeGameplayTags.h"
#include "ZodiacCharacterType.h"

#include "ZodiacCharacterMovementComponent.generated.h"

enum class EZodiacWalkMode : uint8;
class UObject;
struct FFrame;

ZODIAC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_MovementStopped);

UENUM(BlueprintType)
enum EZodiacCustomMovementMode
{
	Move_Custom_None = 0			UMETA(DisplayName="None"),
	Move_Custom_Walking				UMETA(DisplayName="Walking"),
	Move_Custom_Running				UMETA(DisplayName="Running"), 
	Move_Custom_Traversal			UMETA(DisplayName="Traversal")
};

UENUM(BlueprintType, DisplayName = "Movement Input Direction")
enum class EZodiacMovementInputDirection : uint8
{
	None,
	Forward,
	Backward,
	Left,
	Right
};

USTRUCT(BlueprintType)
struct FZodiacMovementInputDirections
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	EZodiacMovementInputDirection PrimaryDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	EZodiacMovementInputDirection SecondaryDirection;

	FZodiacMovementInputDirections()
		: PrimaryDirection(EZodiacMovementInputDirection::None)
		, SecondaryDirection(EZodiacMovementInputDirection::None)
	{
	}
};

/**
 *	Information about the ground under the character.  It only gets updated as needed.
 */
USTRUCT(BlueprintType) 
struct FZodiacCharacterGroundInfo
{
	GENERATED_BODY()

	FZodiacCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};

/**
 *	The base character movement component class used by this project.
 */
UCLASS(Config = Game)
class ZODIAC_API UZodiacCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UZodiacCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	virtual void SimulateMovement(float DeltaTime) override;
	virtual float GetMaxSpeed() const override;
	virtual bool HandlePendingLaunch() override;
	
	// Returns the current ground info.  Calling this will update the ground info if it's out of date.
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	const FZodiacCharacterGroundInfo& GetGroundInfo();

	void SetReplicatedAcceleration(const FVector& InAcceleration);

	// First element return primary direction, second element return secondary direction if exits. It's for diagonal inputs.
	UFUNCTION(BlueprintCallable, Category = "CharacterMovement")
	FZodiacMovementInputDirections GetMovementInputDirection(bool bUseExplicitInputVector = false, FVector InputVector = FVector(0)) const;
	
	void SetExtendedMovementConfig(const FZodiacExtendedMovementConfig& InConfig);

	EZodiacExtendedMovementMode GetExtendedMovementMode() const { return ExtendedMovementMode; }
	void SetExtendedMovementMode(const EZodiacExtendedMovementMode& InMode) { ExtendedMovementMode = InMode; }

protected:
	float CalculateMaxSpeed() const;

public:
	UPROPERTY(EditAnywhere, Category = "Walk Modes")
	FZodiacExtendedMovementConfig ExtendMovementConfig;

protected:
	EZodiacExtendedMovementMode ExtendedMovementMode;
	
	UPROPERTY(EditAnywhere, Category = "Zodiac|Movement")
	TObjectPtr<UCurveFloat> StrafeSpeedMapCurve;
	
	// Cached ground info for the character.  Do not access this directly!  It's only updated when accessed via GetGroundInfo().
	FZodiacCharacterGroundInfo CachedGroundInfo;

	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;
};

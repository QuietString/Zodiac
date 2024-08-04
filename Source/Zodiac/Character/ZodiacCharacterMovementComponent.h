// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "NativeGameplayTags.h"

#include "ZodiacCharacterMovementComponent.generated.h"

class UObject;
struct FFrame;

ZODIAC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_MovementStopped);

UENUM()
enum EZodiacCustomMovementMode
{
	MOVE_Aiming = 1,
	MOVE_Sprinting,
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

	virtual uint8 PackNetworkMovementMode() const override;
	virtual void UnpackNetworkMovementMode(const uint8 ReceivedMode, TEnumAsByte<EMovementMode>& OutMode, uint8& OutCustomMode, TEnumAsByte<EMovementMode>& OutGroundMode) const override;
	virtual void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode = 0) override;
	virtual void SimulateMovement(float DeltaTime) override;
	virtual float GetMaxSpeed() const override;
	virtual bool CanAttemptJump() const override;
	
	// Returns the current ground info.  Calling this will update the ground info if it's out of date.
	UFUNCTION(BlueprintCallable, Category = "Zodiac|CharacterMovement")
	const FZodiacCharacterGroundInfo& GetGroundInfo();

	void SetReplicatedAcceleration(const FVector& InAcceleration);

protected:
	virtual void InitializeComponent() override;
	
protected:
	UPROPERTY(Category="Character Movement: Sprinting", EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxSprintSpeed;

	UPROPERTY(Category="Character Movement: Aiming", EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxWalkSpeedWithAiming;
	
	// Cached ground info for the character.  Do not access this directly!  It's only updated when accessed via GetGroundInfo().
	FZodiacCharacterGroundInfo CachedGroundInfo;

	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;

};

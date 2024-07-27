// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ZodiacAnimInstance.generated.h"

class UCharacterMovementComponent;
class AZodiacHostCharacter;
/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	void UpdateData();
	void SetGait();
	
protected:
	UPROPERTY()
	TObjectPtr<AZodiacHostCharacter> HostCharacter;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CharacterMovement;

protected:
	UPROPERTY()
	FTransform CharacterTransform;

	UPROPERTY()
	FTransform RootTransform;

	UPROPERTY()
	bool HasAcceleration;

	UPROPERTY()
	FVector Acceleration;

	UPROPERTY()
	float AccelerationAmount;

	UPROPERTY()
	bool HasVelocity;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FVector Velocity_LastFrame;

	UPROPERTY()
	FVector VelocityAcceleration;

	UPROPERTY()
	FVector LastNonZeroVelocity;

	UPROPERTY()
	float Speed2D;

	UPROPERTY()
	int32 MMDatabaseLOD;
	
};

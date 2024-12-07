// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility_Sprint.h"
#include "Animation/AnimInstance.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ZodiacHostAnimInstance.generated.h"

class UZodiacCharacterMovementComponent;
class AZodiacCharacter;

UENUM(BlueprintType)
enum EZodiacGait
{
	Gait_Walk,
	Gait_Run,
	Gait_Sprint
};

UCLASS()
class ZODIAC_API UZodiacHostAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	void InitializeWithAbilitySystem(UAbilitySystemComponent* InASC);
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
	bool GetIsAiming() const { return bIsADS; }
	
	void OnStatusChanged(FGameplayTag Tag, bool bHasTag);

protected:
	UFUNCTION(BlueprintCallable)
	void UpdateGait();

private:
	void UpdateLocationData(float DeltaSeconds);
	void UpdateVelocityData();
	void UpdateAccelerationData(float DeltaSeconds);
	void UpdateAimingData();
	
	void UpdateMovementData();

public:
	UPROPERTY(BlueprintReadOnly, meta=(DisplayName = CharacterMovement))
	TObjectPtr<UZodiacCharacterMovementComponent> ZodiacCharMovComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FTransform RootTransform;

	UPROPERTY(BlueprintReadOnly, Category = Location_Data)
	FVector WorldLocation;

	UPROPERTY(BlueprintReadOnly, Category = Location_Data)
	float DisplacementSinceLastUpdate;

	UPROPERTY(BlueprintReadOnly, Category = Location_Data)
	float DisplacementSpeed;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Velocity_Last;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Velocity_LastNonZero;

	// Predicted velocity by trajectory. Doesn't work with root motion.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector FutureVelocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsMoving;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Speed2D;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasVelocity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Acceleration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasAcceleration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float AccelerationAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector AccelerationFromVelocityDiff;

	UPROPERTY(BlueprintReadOnly)
	bool bIsADS;

	UPROPERTY(BlueprintReadOnly)
	bool bIsFocus;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDead;

	UPROPERTY(BlueprintReadOnly)
	bool bIsStun;

	UPROPERTY(BlueprintReadOnly)
	bool bIsTraversal;
	
	UPROPERTY(BlueprintReadOnly, Category = Rotation_Data)
	FRotator WorldRotation;

	UPROPERTY(BlueprintReadOnly, Category = Rotation_Data)
	float YawDeltaSinceLastUpdate;

	// how much angle character should lean toward left or right
	// UPROPERTY(BlueprintReadOnly, Category = Rotation_Data)
	// float AdditiveLeanAngle;

	UPROPERTY( BlueprintReadWrite)
	float AimYaw; 

	UPROPERTY(BlueprintReadWrite)
	float AimPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacGait> Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacGait> Gait_LastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EZodiacExtendedMovementMode ExtendedMovementMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EZodiacExtendedMovementMode ExtendedMovementMode_Last;
	
protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AZodiacCharacter> OwningCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;
};

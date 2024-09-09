// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility_Sprint.h"
#include "Animation/AnimInstance.h"
#include "Character/ZodiacCharacterMovementComponent.h"
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
	
	void UpdateVelocityData();
	void UpdateAccelerationData(float DeltaSeconds);
	void UpdateMovementData();
	bool GetIsAiming() const { return bIsADS; }
	
	void OnStatusChanged(FGameplayTag Tag, bool bHasTag);

protected:
	UFUNCTION(BlueprintCallable)
	void UpdateGait();

public:
	UPROPERTY(BlueprintReadOnly, meta=(DisplayName = CharacterMovement))
	TObjectPtr<UZodiacCharacterMovementComponent> ZodiacCharMovComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FTransform RootTransform;

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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacCustomMovementMode> CustomMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacCustomMovementMode> CustomMovement_Last;
	
	UPROPERTY( BlueprintReadWrite)
	float AimYaw; 

	UPROPERTY(BlueprintReadWrite)
	float AimPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacGait> Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacGait> Gait_LastFrame;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AZodiacCharacter> OwningCharacter;
};

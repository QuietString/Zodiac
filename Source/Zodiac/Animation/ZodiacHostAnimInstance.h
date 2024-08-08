// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility_Sprint.h"
#include "Animation/AnimInstance.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "Character/ZodiacHostCharacter.h"
#include "ZodiacHostAnimInstance.generated.h"

class UZodiacCharacterMovementComponent;

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

	bool GetIsAiming() const { return bIsAimingMovement; }

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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacCustomMovementMode> CustomMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacCustomMovementMode> CustomMovement_Last;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AimYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AimPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacGait> Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacGait> Gait_LastFrame;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AZodiacHostCharacter> HostCharacter;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAimingMovement;
};

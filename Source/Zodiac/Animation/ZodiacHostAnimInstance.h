// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "GameplayEffectTypes.h"
#include "ZodiacHostAnimInstance.generated.h"

struct FAnimUpdateContext;
struct FAnimNodeReference;
class UPoseSearchDatabase;
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

	UFUNCTION(BlueprintCallable)               
	FVector GetTranslationOffset() const;

protected:
	UFUNCTION(BlueprintCallable)
	void UpdateGait();

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = MotionMatching)
	void UpdateMotionMatchingPoseSelection(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	
private:
	void UpdateLocationData(float DeltaSeconds);
	void UpdateRotationData();
	
	void UpdateVelocityData();
	void UpdateAccelerationData(float DeltaSeconds);
	
	void UpdateMovementData(float DeltaSeconds);
	void UpdateAimingData();
	void UpdateHeroData();
	
public:
	UPROPERTY(BlueprintReadOnly, meta=(DisplayName = CharacterMovement))
	TObjectPtr<UZodiacCharacterMovementComponent> ZodiacCharMovComp;

	UPROPERTY(BlueprintReadWrite)
	FTransform RootTransform;

	UPROPERTY(BlueprintReadOnly)
	FTransform CharacterTransform;

	UPROPERTY(BlueprintReadOnly, Category = Location_Data)
	FVector WorldLocation;

	UPROPERTY(BlueprintReadOnly, Category = Location_Data)
	float DisplacementSinceLastUpdate;

	UPROPERTY(BlueprintReadOnly, Category = Location_Data)
	float DisplacementSpeed;

	UPROPERTY(BlueprintReadOnly, Category = Rotation_data)
	FRotator WorldRotation;
	
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
	bool bIsStrafing;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsRunningIntoWall;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Speed2D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Velocity_Data)
	FVector LocalVelocity2D;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasVelocity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Acceleration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Acceleration_Data)
	FVector LocalAcceleration2D;
	
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
	float YawDeltaSinceLastUpdate;

	// how much angle character should lean toward left or right
	// UPROPERTY(BlueprintReadOnly, Category = Rotation_Data)
	// float AdditiveLeanAngle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacGait> Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacGait> Gait_LastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EZodiacExtendedMovementMode ExtendedMovementMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EZodiacExtendedMovementMode ExtendedMovementMode_Last;

	UPROPERTY(transient)
	float MovementAngle;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (ClampMin = "0.0"))
	float InterpSpeed_MaxRootRotationOffset = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float MaxRootRotationOffset_Idle = 170.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float MaxRootRotationOffset_Focus = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float MaxRootRotationOffset_Sprint = 360.f;

	// Allowed max root rotation offset.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Rotation")
	float MaxRootRotationOffset;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Trajectory")
	TArray<TObjectPtr<ACharacter>> ActorsToIgnoreTrajectory;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Aiming")
	float AimYaw;
	
	float AimYaw_Last;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Aiming")
	float AimPitch;
	
protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AZodiacCharacter> OwningCharacter;
	
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<const UPoseSearchDatabase> CurrentSelectedDatabase;

	UPROPERTY(BlueprintReadOnly)
	TArray<FName> CurrentDatabaseTags;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<const UAnimSequence> Debug_SelectedAnimSequence;
#endif
};

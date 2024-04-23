// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimNodeReference.h"
#include "ZodiacAnimInstance.generated.h"

class UAbilitySystemComponent;
class UCharacterMovementComponent;

UENUM()
enum EAnimEnum_CardinalDirection
{
	Front UMETA(DisplayName = "Forward"),
	Backward   UMETA(DisplayName = "Backward"),
	Left      UMETA(DisplayName = "Left"),
	Right   UMETA(DisplayName = "Right")
};

UENUM()
enum EAnimEnum_RootYawOffsetMode
{
	BlendOut UMETA(DisplayName = "BlendOut"),
	Hold UMETA(DisplayName = "Hold"),
	Accumulate UMETA(DisplayName = "Accumulate")
};


/**
 *	The base game animation instance class used by this project.
 */
UCLASS(Config = Game)
class ZODIAC_API UZodiacAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UZodiacAnimInstance(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

	float GetGroundDistance() const;

protected:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UCharacterMovementComponent* GetMovementComponent();

	// Gameplay tags that can be mapped to blueprint variables. The variables will automatically update as the tags are added or removed.
	// These should be used instead of manually querying for the gameplay tags.
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	UPROPERTY(BlueprintReadOnly, Category = "Character State Data")
	float GroundDistance = -1.f;

	bool IsFirstUpdate = true;


/***********************
* GAMEPLAY TAG BINDINGS
************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay_Tag_Bindings)
	bool GameplayTag_IsFiring;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay_Tag_Bindings)
	bool GameplayTag_IsReloading;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay_Tag_Bindings)
	bool GameplayTag_IsDashing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay_Tag_Bindings)
	bool GameplayTag_IsMelee;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay_Tag_Bindings)
	bool GameplayTag_IsADS;

	/***********************
	* LOCATION DATA
	************************/

protected:

	void UpdateLocationData(float DeltaSeconds);

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Location_Data)
		FVector WorldLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Location_Data)
		float DisplacementSinceLastUpdate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Location_Data)
		float DisplacementSpeed;


	/***********************
	* ROTATION DATA
	************************/

protected:

	void UpdateRotationData(float DeltaSeconds);

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rotation_Data)
		FRotator WorldRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rotation_Data)
		float YawDeltaSinceLastUpdate;

	// how much angle character should lean toward left or right
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rotation_Data)
		float AdditiveLeanAngle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rotation_Data)
		float YawDeltaSpeed;


	/***********************
	* VELOCITY DATA
	************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Velocity_Data)
		FVector WorldVelocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Velocity_Data)
		FVector LocalVelocity2D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Velocity_Data)
		float LocalVelocityDirectionAngle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Velocity_Data)
		float LocalVelocityDirectionAngleWithOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Velocity_Data)
		TEnumAsByte<EAnimEnum_CardinalDirection> LocalVelocityDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Velocity_Data)
		TEnumAsByte<EAnimEnum_CardinalDirection> LocalVelocityDirectionNoOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Velocity_Data)
		bool HasVelocity;

private:

	void UpdateVelocityData();

	/***********************
	* ACCELERATION DATA
	************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Acceleration_Data)
	FVector LocalAcceleration2D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Acceleration_Data)
	bool HasAcceleration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Acceleration_Data)
	FVector PivotDirection2D;

private:
	EAnimEnum_CardinalDirection GetOppositeCardinalDirection(EAnimEnum_CardinalDirection CurrentDirection);
	/** calculate a cardinal direction from acceleration to be used for pivots */
	void UpdateAccelerationData();

	/***********************
	* CHARACTER STATE DATA
	************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Characte_State_Data)
	bool IsOnGround;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character_State_Data)
	bool IsCrouching;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character_State_Data)
	bool CrouchStateChange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character_State_Data)
	bool ADSStateChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character_State_Data)
	bool WasADSLastUpdate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character_State_Data)
	float TimeSinceFiredWeapon = 9999.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character_State_Data)
	bool IsJumping;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character_State_Data)
	bool IsFalling;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character_State_Data)
	float TimeToJumpApex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character_State_Data)
	bool IsRunningIntoWall;

private:

	void UpdateCharacterStateData(float DeltaSeconds);

	/***********************
	* BLEND WEIGHT DATA
	************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Blend_Weight_Data)
		float UpperBodyDynamicAdditiveWeight;

private:

	void UpdateBlendWeightData(float DeltaSeconds);

/***********************
* TURN IN PLACE DATA
************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Turn_In_Place)
		float RootYawOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Turn_In_Place)
		FFloatSpringState RootYawOffsetSpringState;

	// remaining angle to yaw
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Turn_In_Place)
		float TurnYawCurveValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Turn_In_Place)
		TEnumAsByte<EAnimEnum_RootYawOffsetMode> RootYawOffsetMode = EAnimEnum_RootYawOffsetMode::BlendOut;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Turn_In_Place)
		FVector2D RootYawOffsetAngleClamp = FVector2D(-120.f, 100.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Turn_In_Place)
		FVector2D RootYawOffsetAngleClampCrouched = FVector2D(-90.f, 80.f);

private:
	// when the yaw offset gets too big, trigger TurnInPlace Animations to rotate the character back.
	void ProcessTurnYawCurve();

	void SetRootYawOffset(float InRootYawOffset);

	void UpdateRootYawOffset(float DeltaSeconds);

	/***********************
	* LOCOMOTION SM DATA
	************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Locomotion_SM_Data)
		TEnumAsByte<EAnimEnum_CardinalDirection> StartDirection = EAnimEnum_CardinalDirection::Front;

	// direction of inertia
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Locomotion_SM_Data)
		TEnumAsByte<EAnimEnum_CardinalDirection> PivotInitialDirection = EAnimEnum_CardinalDirection::Front;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Locomotion_SM_Data)
		float LastPivotTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Locomotion_SM_Data)
		TEnumAsByte<EAnimEnum_CardinalDirection> CardinalDirectionFromAcceleration = EAnimEnum_CardinalDirection::Front;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = Helper_Functions)
		bool IsMovingPerpendicularToInitialPivot() const;


	/***********************
	* AIMING DATA
	************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aiming_Data)
	float AimYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aiming_Data)
	float AimPitch;

private:

	void UpdateAimingData();

/***********************
* LINKED LAYER DATA
************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Linked_Layer_Data)
		bool LinkedLayerChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Linked_Layer_Data)
		UAnimInstance* LastLinkedLayer;


/***********************
* STATE NODE BINDING FUNCTIONS
************************/

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = State_Node_Functions)
	void UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = State_Node_Functions)
	void SetUpStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = State_Node_Functions)
	void UpdateStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = State_Node_Functions)
	void UpdateStopState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = State_Node_Functions)
	void SetupPivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = State_Node_Functions)
	void UpdatePivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = State_Node_Functions)
	void UpdateLocomotionStateMachine(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);


/***********************
* LOCOMOTION TRANSITION RULES
************************/

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, Category = TRANSITION_RULES))
	bool IdleToStart() const;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, Category = TRANSITION_RULES))
	bool StartToCycle3();

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, Category = TRANSITION_RULES))
	bool StartToCycle() const;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, Category = TRANSITION_RULES))
	bool StopToIdle() const;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, Category = TRANSITION_RULES))
	bool StopRule() const;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, Category = TRANSITION_RULES))
	bool PivotToCycle() const;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, Category = TRANSITION_RULES))
	bool PivotSourcesToPivot() const;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, Category = TRANSITION_RULES))
	bool JumpStartLoopToJumpApex() const;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, Category = TRANSITION_RULES))
	bool FallLoopToFallLand() const;

/***********************
* OTHERS
************************/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Setting)
		float CardinalDirectionDeadZone = 10.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool DisableControlRig;

private:

	/** guesses if the character is running into a wall
	* by checking if there's a large angle between velocity and acceleration */
	void UpdateWallDetectionHeuristic();

	void UpdateJumpFallData();

	EAnimEnum_CardinalDirection SelectCardinalDirectionFromAngle(
	float Angle, float DeadZone, EAnimEnum_CardinalDirection CurrentDirection, bool UseCurrentDirection);
};

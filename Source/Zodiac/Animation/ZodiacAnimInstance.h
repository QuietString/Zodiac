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

class AZodiacPlayerCharacter;
class UAbilitySystemComponent;
class UCharacterMovementComponent;

UENUM(BlueprintType)
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

USTRUCT(BlueprintType)
struct FAnimStruct_CardinalDirections
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* Forward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* Backward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* Left;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimSequence* Right;

	TArray<UAnimSequence*> GetAnimSequences()
	{
		return { Forward, Backward, Left, Right };
	}
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
	
	virtual void InitializePropertyMap();
	
	float GetGroundDistance() const;

protected:
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
	UCharacterMovementComponent* GetMovementComponent() const;
	AZodiacPlayerCharacter* GetZodiacPlayerCharacter() const;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay_Tag_Bindings)
	bool GameplayTag_IsSprinting;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay_Tag_Bindings)
	bool GameplayTag_Test;
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
	bool ADSStateChanged;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character_State_Data)
	bool WasADSLastUpdate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character_State_Data)
	bool SprintStateChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character_State_Data)
	bool bWasSprintingLastUpdate;

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
	bool TransitionRule_PivotToCycle() const;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe, Category = TRANSITION_RULES))
	bool TransitionRule_PivotSourcesToPivot() const;

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

/***********************
* DISTANCE MATCHING
************************/

private:

	FName LocomotionDistanceCurveName = FName(TEXT("DistanceCurve"));

	bool ShouldDistanceMatchStop() const;

	
	/***********************
	* STRIDE WARPING
	************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PoseWarping)
	float StrideWarpingStartAlpha = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PoseWarping)
	float StrideWarpingPivotAlpha = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PoseWarping)
	float StrideWarpingCycleAlpha = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = Settings)
	float StrideWarpingBlendInDurationScaled = 0.2f;
	
	UPROPERTY(EditDefaultsOnly, Category = Settings)
	float StrideWarpingBlendInStartOffset = 0.15f;
	
/***********************
* NODE FUNCTIONS
************************/

protected:

	// Stops
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Node_Functions)
	void SetupStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Node_Functions)
	void UpdateStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	float GetPredictedStopDistance() const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Stops")
	FAnimStruct_CardinalDirections WalkStopCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Stops")
	FAnimStruct_CardinalDirections JogStopCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Stops")
	FAnimStruct_CardinalDirections SprintStopCardinals;
	
	// Starts
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Node_Functions)
	void SetUpStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Node_Functions)
	void UpdateStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Starts")
	FAnimStruct_CardinalDirections WalkStartCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Starts")
	FAnimStruct_CardinalDirections JogStartCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Starts")
	FAnimStruct_CardinalDirections SprintStartCardinals;

	// Pivots

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Node_Functions)
	void SetupPivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node_Functions);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Node_Functions)
	void UpdatePivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pivots)
	FVector PivotStartingAcceleration;
	
	float TimeAtPivotStop = 0.f;

	// Select anim sequence considering ADS
	UAnimSequence* GetDesiredPivotSequence(EAnimEnum_CardinalDirection InDirection);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Pivots")
	FAnimStruct_CardinalDirections WalkPivotCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Pivots")
	FAnimStruct_CardinalDirections JogPivotCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Pivots")
	FAnimStruct_CardinalDirections SprintPivotCardinals;

	// Cycles
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Node_Functions)
	void UpdateCycleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	float GetOrientationAngle() const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Cycles")
	FAnimStruct_CardinalDirections WalkCardinals;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Cycles")
	FAnimStruct_CardinalDirections JogCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Cycles")
	FAnimStruct_CardinalDirections SprintCardinals;

	// Turn in Place

	// reset explicit time
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Turn_In_Place)
	void SetUpTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Turn_In_Place)
	void UpdateTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Turn_In_Place)
	void SetUpTurnInPlaceRotationState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Turn_In_Place)
	void SetUpTurnInPlaceRecoveryState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Turn_In_Place)
	void UpdateTurnInPlaceRecoveryAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	// select anim sequence considering direction and crouch
	UAnimSequence* SelectTurnInPlaceAnimation(float Direction) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Turn_In_Place)
	float TurnInPlaceAnimTime = 0.f;
	
	float TurnInPlaceRotationDirection = 0.f;
	float TurnInPlaceRecoveryDirection = 0.f;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Turn in Place")
	UAnimSequence* TurnInPlaceLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Turn in Place")
	UAnimSequence* TurnInPlaceRight;

	////////////////////////////
	///
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true), Category = Settings)
	FVector2D PlayRateClampStartsPivots = FVector2D(0.6f, 5.f);

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true), Category = Settings)
	FVector2D PlayRateClampCycle = FVector2D(0.8f, 1.2f);
};

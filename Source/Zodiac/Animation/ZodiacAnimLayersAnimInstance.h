// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimNodeReference.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "ZodiacAnimInstance.h"
#include "ZodiacAnimLayersAnimInstance.generated.h"


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
 * 
 */
UCLASS()
class ZODIAC_API UZodiacAnimLayersAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	UZodiacAnimInstance* GetMainAnimBPThreadSafe() const;

	UCharacterMovementComponent* GetMovementComponent() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UZodiacAnimInstance* MainAnimBP;
	/***********************
	* ANIM SETS
	************************/

	//Idle

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Idle")
	UAnimSequence* IdleADS;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Idle")
	UAnimSequence* IdleHipfire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Idle")
	TArray<UAnimSequence*> IdleBreaks;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Idle")
	UAnimSequence* CrouchIdle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Idle")
	UAnimSequence* CrouchIdleEntry;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Idle")
	UAnimSequence* CrouchIdleExit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Idle")
	UAnimSequence* LeftHandPoseOverride;

	// Starts

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Starts")
	FAnimStruct_CardinalDirections ADSStartCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Starts")
	FAnimStruct_CardinalDirections JogStartCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Starts")
	FAnimStruct_CardinalDirections CrouchStartCardinals;

	// Stops

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Stops")
	FAnimStruct_CardinalDirections ADSStopCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Stops")
	FAnimStruct_CardinalDirections JogStopCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Stops")
	FAnimStruct_CardinalDirections CrouchStopCardinals;

	// Pivots

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Pivots")
	FAnimStruct_CardinalDirections ADSPivotCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Pivots")
	FAnimStruct_CardinalDirections JogPivotCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Pivots")
	FAnimStruct_CardinalDirections CrouchPivotCardinals;

	// Turn in Place

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Turn in Place")
	UAnimSequence* TurnInPlaceLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Turn in Place")
	UAnimSequence* TurnInPlaceRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Turn in Place")
	UAnimSequence* CrouchTurnInPlaceLeft;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Turn in Place")
	UAnimSequence* CrouchTurnInPlaceRight;

	// Walk

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Walk")
	FAnimStruct_CardinalDirections WalkCardinals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Walk")
	FAnimStruct_CardinalDirections CrouchWalkCardinals;

	// Jog

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Jog")
	FAnimStruct_CardinalDirections JogCardinals;

	// Aiming

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Aiming")
	UAnimSequence* AimHipFirePose;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Aiming")
	UAnimSequence* AimHipFirePoseCrouch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Aiming")
	UAimOffsetBlendSpace* IdleAimOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Aiming")
	UAimOffsetBlendSpace* RelaxedAimOffset;

	// Jump

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Jump")
	UAnimSequence* JumpStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Jump")
	UAnimSequence* JumpApex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Jump")
	UAnimSequence* JumpFallLand;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Jump")
	UAnimSequence* JumpStartLoop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Jump")
	UAnimSequence* JumpFallLoop;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim Set - Jump")
	UAnimSequence* JumpRecoveryAdditive;
	

	/***********************
	* IDLE STATE
	************************/

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Idle)
	void SetUpIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Idle)
	void UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	// select anim sequence for idle state
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Idle)
	void UpdateIdleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	/***********************
	* IDLE BREAKS STATE
	************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Idle_Breaks)
	float TimeUntilNextIdleBreak;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = Idle_Breaks)
	bool CanPlayIdleBreak() const;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Idle)
	void SetUpIdleBreakAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Idle)
	void SetUpIdleTransition(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

private:

	void ResetIdleBreakTransitionLogic();
	void ProcessIdleBreakTransitionLogic();
	void ChooseIdleBreakDelayTime();

	bool WantsIdleBreak;
	int CurrentIdleBreakIndex;
	float IdleBreakDelayTime;

	/***********************
	* TURN IN PLACE STATE
	************************/

public:

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Turn_In_Place)
	float TurnInPlaceAnimTime = 0.f;

private:

	float TurnInPlaceRotationDirection = 0.f;
	float TurnInPlaceRecoveryDirection = 0.f;

	// select anim sequence considering direction and crouch
	UAnimSequence* SelectTurnInPlaceAnimation(float Direction) const;

	/***********************
	* START STATE
	************************/

public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Anim_Node_Functions)
	void SetUpStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Anim_Node_Functions)
	void UpdateStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);


	/***********************
	* CYCLE STATE
	************************/

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Anim_Node_Functions)
	void UpdateCycleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	
	/***********************
	* STOP STATE
	************************/

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Stop)
	void SetupStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Stop)
	void UpdateStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

private:
	float GetPredictedStopDistance() const;


	/***********************
	* PIVOT STATE
	************************/

public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Pivots)
	void SetupPivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Pivots)
	void UpdatePivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pivots)
	FVector PivotStartingAcceleration;

private:

	float TimeAtPivotStop = 0.f;

	// Select anim sequence considering ADS and crouch
	UAnimSequence* GetDesiredPivotSequence(EAnimEnum_CardinalDirection InDirection);

	/***********************
	* JUMP STATES
	************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Jump_State)
	float LandRecoveryAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Jump_State)
	float TimeFalling;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Jump_State)
	void SetUpFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Jump_State)
	void UpdateFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Jump_State)
	void LandRecoveryStart(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

private:

	void UpdateJumpFallData();

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
	

	/***********************
	* DISTANCE MATCHING
	************************/

private:

	FName LocomotionDistanceCurveName = FName(TEXT("Distance"));

	bool ShouldDistanceMatchStop() const;

	/***********************
	* LEFT HAND POSE
	************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Left_Hand_Pose)
	float LeftHandPoseOverrideWeight;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Left_Hand_Pose)
	void SetLeftHandPoseOverrideWeight(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	
	
	/***********************
	* SKEL CONTROL DATA
	************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Skel_Control_Data)
	float HandFKWeight = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Skel_Control_Data)
	float HandIKRightAlpha = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Skel_Control_Data)
	float HandIKLeftAlpha = 1.f;

private:
	
	void UpdateSkelControlData();

	/***********************
	* BLEND WEIGHT DATA
	************************/

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Blend_Weight_Data)
	float AimOffsetBlendWeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Blend_Weight_Data)
	float HipFireUpperBodyOverrideWeight;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe), Category = Blend_Weight_Data)
	void UpdateHipFireRaiseWeaponPose(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

private:

	void UpdateBlendWeightData();

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true), Category = Settings)
	bool DisableHandIK = false;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true), Category = Settings)
	bool EnableLeftHandPoseOverride = false;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true), Category = Settings)
	FName JumpDistanceCurveName = FName("Distance");
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true), Category = Settings)
	FVector2D PlayRateClampCycle = FVector2D(0.8f, 1.2f);

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true), Category = Settings)
	FVector2D PlayRateClampStartsPivots = FVector2D(0.6f, 5.f);

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true), Category = Settings)
	bool RaiseWeaponAfterFiringWhenCrouched = false;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true), Category = Settings)
	float RaiseWeaponAfterFiringDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true), Category = Settings)
	float StrideWarpingBlendInDurationScaled = 0.2f;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true), Category = Settings)
	float StrideWarpingBlendInStartOffset = 0.15f;
};

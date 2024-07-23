// the.quiet.string@gmail.com

#include "ZodiacAnimLayersAnimInstance.h"
#include "SequencePlayerLibrary.h"
#include "SequenceEvaluatorLibrary.h"
#include "AnimCharacterMovementLibrary.h"
#include "AnimDistanceMatchingLibrary.h"
#include "AnimationStateMachineLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

void UZodiacAnimLayersAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	/*if (const USkeletalMeshComponent* OwningComponent = GetOwningComponent())
	{
		UAnimInstance* AnimInstance = OwningComponent->GetAnimInstance();
		MainAnimBP = Cast<UZodiacAnimInstance>(AnimInstance);
	}*/
}

void UZodiacAnimLayersAnimInstance::NativeThreadSafeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (GetMainAnimBPThreadSafe())
	{
		UpdateBlendWeightData();
		UpdateJumpFallData();
		UpdateSkelControlData();
	}
	/*if (MainAnimBP)
	{
		UE_LOG(LogTemp, Warning, TEXT("displacement speed: %f"), MainAnimBP->DisplacementSpeed);
	}*/
}

UZodiacAnimInstance* UZodiacAnimLayersAnimInstance::GetMainAnimBPThreadSafe() const
{
	UAnimInstance* AnimInstance = GetOwningComponent()->GetAnimInstance();
	return Cast<UZodiacAnimInstance>(AnimInstance);
}

UCharacterMovementComponent* UZodiacAnimLayersAnimInstance::GetMovementComponent() const
{
	UPawnMovementComponent* PawnMovementComponent = TryGetPawnOwner()->GetMovementComponent();
	return Cast<UCharacterMovementComponent>(PawnMovementComponent);
}

void UZodiacAnimLayersAnimInstance::SetUpIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	ChooseIdleBreakDelayTime();
	ResetIdleBreakTransitionLogic();
}

void UZodiacAnimLayersAnimInstance::UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FAnimationStateResultReference AnimationState;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationState, Result);

	const bool IsBlendingOut = UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationState);
	if (!IsBlendingOut) ProcessIdleBreakTransitionLogic();
}

void UZodiacAnimLayersAnimInstance::UpdateIdleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	UAnimSequence* SequenceToPlay = (GetMainAnimBPThreadSafe()->GameplayTag_IsADS) ? IdleADS : IdleHipfire;

	EAnimNodeReferenceConversionResult Result;
	const FSequencePlayerReference SequencePlayerRef = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayerRef, SequenceToPlay, 0.2f);
}

bool UZodiacAnimLayersAnimInstance::CanPlayIdleBreak() const
{
	const bool HasIdleAnims = IdleBreaks.Num() > 0;
	const bool CanPlayIdleBreak = !(GetMainAnimBPThreadSafe()->GameplayTag_IsADS
		|| GetMainAnimBPThreadSafe()->GameplayTag_IsFiring || GetMainAnimBPThreadSafe()->IsAnyMontagePlaying()
		|| GetMainAnimBPThreadSafe()->HasVelocity || GetMainAnimBPThreadSafe()->IsJumping);

	return CanPlayIdleBreak && HasIdleAnims;
}

void UZodiacAnimLayersAnimInstance::SetUpIdleBreakAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequencePlayerReference SequencePlayerRef = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);
	
	USequencePlayerLibrary::SetSequence(SequencePlayerRef, IdleBreaks[CurrentIdleBreakIndex]);
	if (++CurrentIdleBreakIndex >= IdleBreaks.Num()) CurrentIdleBreakIndex = 0;
}

void UZodiacAnimLayersAnimInstance::SetUpIdleTransition(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequencePlayerReference SequencePlayerRef = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	UAnimSequence* SequenceToPlay = CrouchIdleExit;
	USequencePlayerLibrary::SetSequence(SequencePlayerRef, SequenceToPlay);
}

void UZodiacAnimLayersAnimInstance::ResetIdleBreakTransitionLogic()
{
	TimeUntilNextIdleBreak = IdleBreakDelayTime;
}

void UZodiacAnimLayersAnimInstance::ProcessIdleBreakTransitionLogic()
{
	if (CanPlayIdleBreak())
	{
		TimeUntilNextIdleBreak = TimeUntilNextIdleBreak - GetDeltaSeconds();
	}
	else
	{
		ResetIdleBreakTransitionLogic();
	}
}

void UZodiacAnimLayersAnimInstance::ChooseIdleBreakDelayTime()
{
	const float LocationSum = FMath::Abs(GetMainAnimBPThreadSafe()->WorldLocation.X + GetMainAnimBPThreadSafe()->WorldLocation.Y);
	IdleBreakDelayTime = static_cast<float>(FMath::TruncToInt(LocationSum) % 10 + 6);
}

void UZodiacAnimLayersAnimInstance::SetUpTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	TurnInPlaceAnimTime = 0.f;

	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef =
		USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluatorRef, 0.f);
}

void UZodiacAnimLayersAnimInstance::UpdateTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef =
		USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	UAnimSequence* SequenceToPlay = SelectTurnInPlaceAnimation(TurnInPlaceRotationDirection);
	USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEvaluatorRef, SequenceToPlay, 0.2f);

	TurnInPlaceAnimTime += GetDeltaSeconds();
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluatorRef, TurnInPlaceAnimTime);
}

void UZodiacAnimLayersAnimInstance::SetUpTurnInPlaceRotationState(const FAnimUpdateContext& Context,
	const FAnimNodeReference& Node)
{
	TurnInPlaceRotationDirection = -FMath::Sign(GetMainAnimBPThreadSafe()->RootYawOffset);
}

void UZodiacAnimLayersAnimInstance::SetUpTurnInPlaceRecoveryState(const FAnimUpdateContext& Context,
	const FAnimNodeReference& Node)
{
	TurnInPlaceRecoveryDirection = TurnInPlaceRotationDirection;
}

void UZodiacAnimLayersAnimInstance::UpdateTurnInPlaceRecoveryAnim(const FAnimUpdateContext& Context,
	const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequencePlayerReference SequencePlayerRef = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	UAnimSequence* SequenceToPlay = SelectTurnInPlaceAnimation(TurnInPlaceRecoveryDirection);
	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayerRef, SequenceToPlay, 0.2f);
}

void UZodiacAnimLayersAnimInstance::SetUpStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	const int Index = GetMainAnimBPThreadSafe()->LocalVelocityDirection;
	UAnimSequence* ADSAnim = ADSStartCardinals.GetAnimSequences()[Index];
	UAnimSequence* JogAnim = JogStartCardinals.GetAnimSequences()[Index];
	UAnimSequence* CrouchAnim = CrouchStartCardinals.GetAnimSequences()[Index];

	UAnimSequence* SequenceToPlay = GetMainAnimBPThreadSafe()->GameplayTag_IsADS ? ADSAnim : JogAnim;

	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEvaluatorRef =
		USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	SequenceEvaluatorRef = USequenceEvaluatorLibrary::SetSequence(SequenceEvaluatorRef, SequenceToPlay);

	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluatorRef, 0.f);
	StrideWarpingStartAlpha = 0.f;
}

void UZodiacAnimLayersAnimInstance::UpdateStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef =
		USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	const float ExplicitTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEvaluatorRef);
	const float Alpha = ExplicitTime - StrideWarpingStartAlpha;

	const UE::Math::TVector2<float> InRange = UE::Math::TVector2<float>(0.f, StrideWarpingBlendInDurationScaled);
	const UE::Math::TVector2<float> OutRange = UE::Math::TVector2<float>(0.f, 1.f);
	StrideWarpingStartAlpha = FMath::GetMappedRangeValueClamped(InRange, OutRange, Alpha);

	const float ClampMin = FMath::Lerp(StrideWarpingBlendInDurationScaled, PlayRateClampStartsPivots.X, StrideWarpingStartAlpha);
	const float ClampMax = PlayRateClampStartsPivots.Y;
	const FVector2D PlayRateClamp = FVector2D(ClampMin, ClampMax);

	UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(
		Context, SequenceEvaluatorRef, GetMainAnimBPThreadSafe()->DisplacementSinceLastUpdate, LocomotionDistanceCurveName, PlayRateClamp);
}

void UZodiacAnimLayersAnimInstance::UpdateCycleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	const int Index = GetMainAnimBPThreadSafe()->LocalVelocityDirection;
	UAnimSequence* JogAnim = JogCardinals.GetAnimSequences()[Index];
	UAnimSequence* ADSAnim = WalkCardinals.GetAnimSequences()[Index];
	UAnimSequence* CrouchAnim = CrouchWalkCardinals.GetAnimSequences()[Index];

	UAnimSequence* SequenceToPlay = GetMainAnimBPThreadSafe()->GameplayTag_IsADS ? ADSAnim : JogAnim;

	EAnimNodeReferenceConversionResult Result;
	const FSequencePlayerReference SequencePlayerRef = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);
	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayerRef, SequenceToPlay, 0.2f);

	const float SpeedToMatch = GetMainAnimBPThreadSafe()->DisplacementSpeed;
	UAnimDistanceMatchingLibrary::SetPlayrateToMatchSpeed(SequencePlayerRef, SpeedToMatch, PlayRateClampCycle);

	const float LerpTarget = GetMainAnimBPThreadSafe()->IsRunningIntoWall ? 0.5f : 1.f;
	StrideWarpingCycleAlpha = FMath::FInterpTo(StrideWarpingCycleAlpha, LerpTarget, GetDeltaSeconds(), 10.f);
}

UAnimSequence* UZodiacAnimLayersAnimInstance::SelectTurnInPlaceAnimation(const float Direction) const
{
	UAnimSequence* TurnLeft = TurnInPlaceLeft;
	UAnimSequence* TurnRight = TurnInPlaceRight;

	return (Direction > 0.f) ? TurnRight : TurnLeft;
}

void UZodiacAnimLayersAnimInstance::SetupStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	const int Index = GetMainAnimBPThreadSafe()->LocalVelocityDirection;
	UAnimSequence* JogAnim = JogStopCardinals.GetAnimSequences()[Index];
	UAnimSequence* ADSAnim = ADSStopCardinals.GetAnimSequences()[Index];
	UAnimSequence* CrouchAnim = CrouchStopCardinals.GetAnimSequences()[Index];
	UAnimSequence* SequenceToPlay = GetMainAnimBPThreadSafe()->GameplayTag_IsADS ? ADSAnim : JogAnim;

	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef =
		USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	USequenceEvaluatorLibrary::SetSequence(SequenceEvaluatorRef, SequenceToPlay);

	if (!ShouldDistanceMatchStop())
	{
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluatorRef, 0.f, LocomotionDistanceCurveName);
	}
}

void UZodiacAnimLayersAnimInstance::UpdateStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef =
		USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	if (float DistanceToMatch; ShouldDistanceMatchStop() && (DistanceToMatch = GetPredictedStopDistance()) > 0.f)
	{
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluatorRef, DistanceToMatch, LocomotionDistanceCurveName);
	}
	else
	{
		USequenceEvaluatorLibrary::AdvanceTime(Context, SequenceEvaluatorRef, 1.f);
	}
}

float UZodiacAnimLayersAnimInstance::GetPredictedStopDistance() const
{
	const FVector StopLocation = UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(
		GetMovementComponent()->GetLastUpdateVelocity(),
		GetMovementComponent()->bUseSeparateBrakingFriction,
		GetMovementComponent()->BrakingFriction,
		GetMovementComponent()->GroundFriction,
		GetMovementComponent()->BrakingFrictionFactor,
		GetMovementComponent()->BrakingDecelerationWalking);

	const float PredictedStopDistance = UKismetMathLibrary::VSizeXY(StopLocation);

	return PredictedStopDistance;
}

void UZodiacAnimLayersAnimInstance::SetupPivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	PivotStartingAcceleration = GetMainAnimBPThreadSafe()->LocalAcceleration2D;

	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef =
		USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	UAnimSequence* SequenceToPlay = GetDesiredPivotSequence(GetMainAnimBPThreadSafe()->CardinalDirectionFromAcceleration);
	USequenceEvaluatorLibrary::SetSequence(SequenceEvaluatorRef, SequenceToPlay);
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluatorRef, 0.f);

	StrideWarpingPivotAlpha = 0.f;
	TimeAtPivotStop = 0.f;
	GetMainAnimBPThreadSafe()->LastPivotTime = 0.2f;
}

void UZodiacAnimLayersAnimInstance::UpdatePivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef =
		USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	const float ExplicitTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEvaluatorRef);

	// allow switching the selected pivot for a short duration at the beginning
	if (GetMainAnimBPThreadSafe()->LastPivotTime > 0.f)
	{
		UAnimSequence* NewDesiredAnimSequence = GetDesiredPivotSequence(GetMainAnimBPThreadSafe()->CardinalDirectionFromAcceleration);
		const UAnimSequenceBase* CurrentSequence = USequenceEvaluatorLibrary::GetSequence(SequenceEvaluatorRef);
		if (NewDesiredAnimSequence != CurrentSequence)
		{
			USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEvaluatorRef, NewDesiredAnimSequence, 0.2f);
			PivotStartingAcceleration = GetMainAnimBPThreadSafe()->LocalAcceleration2D;
		}
	}

	// does acceleration oppose velocity?
	const float DotProduct = FVector::DotProduct(GetMainAnimBPThreadSafe()->LocalVelocity2D, GetMainAnimBPThreadSafe()->LocalAcceleration2D);
	if (DotProduct < 0.f)
	{
		// while acceleration oppose velocity, the character is still approaching the pivot point, so we distance match to that point
		const FVector PivotLocation = UAnimCharacterMovementLibrary::PredictGroundMovementPivotLocation(
			GetMovementComponent()->GetCurrentAcceleration(), GetMovementComponent()->GetLastUpdateVelocity(), GetMovementComponent()->GroundFriction);
		const float DistanceToTarget = UKismetMathLibrary::VSizeXY(PivotLocation);
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluatorRef, DistanceToTarget, LocomotionDistanceCurveName);
		TimeAtPivotStop = ExplicitTime;
	}
	else
	{
		// we want the blend in to start after we've already stopped, and just started accelerating.
		const float Alpha = ExplicitTime - TimeAtPivotStop - StrideWarpingBlendInStartOffset;
		const UE::Math::TVector2 InRange = UE::Math::TVector2(0.f, StrideWarpingBlendInDurationScaled);
		const UE::Math::TVector2 OutRange = UE::Math::TVector2(0.f, 1.f);
		StrideWarpingPivotAlpha = FMath::GetMappedRangeValueClamped(InRange, OutRange, Alpha);

		const float ClampMin = FMath::Lerp(0.2f, PlayRateClampStartsPivots.X, StrideWarpingPivotAlpha);
		const float ClampMax = PlayRateClampStartsPivots.Y;
		const FVector2D PlayRateClamp = FVector2D(ClampMin, ClampMax);

		UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(
			Context, SequenceEvaluatorRef, GetMainAnimBPThreadSafe()->DisplacementSinceLastUpdate, LocomotionDistanceCurveName, PlayRateClamp);
	}
}

UAnimSequence* UZodiacAnimLayersAnimInstance::GetDesiredPivotSequence(const EAnimEnum_CardinalDirection InDirection)
{
	UAnimSequence* JogAnim = JogPivotCardinals.GetAnimSequences()[InDirection];
	UAnimSequence* ADSAnim = ADSPivotCardinals.GetAnimSequences()[InDirection];
	UAnimSequence* CrouchAnim = CrouchPivotCardinals.GetAnimSequences()[InDirection];

	return GetMainAnimBPThreadSafe()->GameplayTag_IsADS ? ADSAnim : JogAnim;
}

void UZodiacAnimLayersAnimInstance::SetUpFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef =
		USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluatorRef, 0.f);
}

void UZodiacAnimLayersAnimInstance::UpdateFallLandAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef =
		USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluatorRef, GetMainAnimBPThreadSafe()->GetGroundDistance(), JumpDistanceCurveName);
}

void UZodiacAnimLayersAnimInstance::LandRecoveryStart(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	const UE::Math::TVector2 InRange = UE::Math::TVector2(0.f, 0.4f);
	const UE::Math::TVector2 OutRange = UE::Math::TVector2(0.1f, 1.0f);
	LandRecoveryAlpha = FMath::GetMappedRangeValueClamped(InRange, OutRange, TimeFalling);
}

void UZodiacAnimLayersAnimInstance::UpdateJumpFallData()
{
	if (GetMainAnimBPThreadSafe()->IsFalling)
	{
		TimeFalling += GetDeltaSeconds();
	}
	else if (GetMainAnimBPThreadSafe()->IsJumping)
	{
		TimeFalling = 0.f;
	}
}

bool UZodiacAnimLayersAnimInstance::ShouldDistanceMatchStop() const
{
	return GetMainAnimBPThreadSafe()->HasVelocity && !GetMainAnimBPThreadSafe()->HasAcceleration;
}

void UZodiacAnimLayersAnimInstance::SetLeftHandPoseOverrideWeight(const FAnimUpdateContext& Context,
	const FAnimNodeReference& Node)
{
	const float OverrideOffset = (EnableLeftHandPoseOverride) ? 1.f : 0.f;
	const float CurveValue = GetCurveValue(FName("DisableLeftHandPoseOverride"));
	LeftHandPoseOverrideWeight = FMath::Clamp(OverrideOffset - CurveValue, 0.f, 1.f);
}

void UZodiacAnimLayersAnimInstance::UpdateSkelControlData()
{
	const float AlphaDefault = DisableHandIK ? 0.f : 1.f;
	HandIKRightAlpha = FMath::Clamp(AlphaDefault - GetCurveValue("DisableRHandIK"), 0.f, 1.f);
	HandIKLeftAlpha = FMath::Clamp(AlphaDefault - GetCurveValue("DisableLHandIK"), 0.f, 1.f);
}

void UZodiacAnimLayersAnimInstance::UpdateHipFireRaiseWeaponPose(const FAnimUpdateContext& Context,
	const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef =
		USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	UAnimSequence* SequenceToPlay = AimHipFirePose;
	USequenceEvaluatorLibrary::SetSequence(SequenceEvaluatorRef, SequenceToPlay);
}

void UZodiacAnimLayersAnimInstance::UpdateBlendWeightData()
{
	// hip fire override weight
	const bool CrouchCondition = !RaiseWeaponAfterFiringWhenCrouched;
	const bool ADSCondition = GetMainAnimBPThreadSafe()->GameplayTag_IsADS && GetMainAnimBPThreadSafe()->IsOnGround;

	if (CrouchCondition || ADSCondition)
	{
		HipFireUpperBodyOverrideWeight = 0.f;
		AimOffsetBlendWeight = 1.f;
	}
	else
	{
		const bool ShouldNotRaiseWeapon = GetMainAnimBPThreadSafe()->TimeSinceFiredWeapon < RaiseWeaponAfterFiringDuration;
		const bool IsADSInCrouchedOrAir = GetMainAnimBPThreadSafe()->GameplayTag_IsADS && (!GetMainAnimBPThreadSafe()->IsOnGround);
		const bool ShouldApplyHipFireOverride = GetCurveValue("applyHipfireOverridePose") > 0.f;
		if (ShouldNotRaiseWeapon || IsADSInCrouchedOrAir || ShouldApplyHipFireOverride)
		{
			HipFireUpperBodyOverrideWeight = 1.f;
			AimOffsetBlendWeight = 1.f;
		}
		else
		{
			HipFireUpperBodyOverrideWeight = FMath::FInterpTo(HipFireUpperBodyOverrideWeight, 0.f, GetDeltaSeconds(), 1.f);

			// use aiming offset when we are idle or we have root yaw offset
			// use relaxed aim offset during regular motion
			const bool ShouldUseRelaxedAimOffset = FMath::Abs(GetMainAnimBPThreadSafe()->RootYawOffset) < 10.f &&
				GetMainAnimBPThreadSafe()->HasAcceleration;
			const float InterpTarget = ShouldUseRelaxedAimOffset ? HipFireUpperBodyOverrideWeight : 1.f;
			AimOffsetBlendWeight = FMath::FInterpTo(AimOffsetBlendWeight, InterpTarget, GetDeltaSeconds(), 10.f);
		}

	}
}
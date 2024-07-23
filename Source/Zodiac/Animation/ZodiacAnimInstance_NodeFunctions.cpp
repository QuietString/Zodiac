// the.quiet.string@gmail.com

#include "AnimCharacterMovementLibrary.h"
#include "AnimDistanceMatchingLibrary.h"
#include "SequenceEvaluatorLibrary.h"
#include "ZodiacAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"


void UZodiacAnimInstance::SetupStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	const int Index = LocalVelocityDirection;
	UAnimSequence* JogAnim = JogStopCardinals.GetAnimSequences()[Index];
	UAnimSequence* ADSAnim = WalkStopCardinals.GetAnimSequences()[Index];
	UAnimSequence* SequenceToPlay = GameplayTag_IsADS ? ADSAnim : JogAnim;
	
	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	USequenceEvaluatorLibrary::SetSequence(SequenceEvaluatorRef, SequenceToPlay);

	if (!ShouldDistanceMatchStop())
	{
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluatorRef, 0.f, LocomotionDistanceCurveName);
	}
}

void UZodiacAnimInstance::UpdateStopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	if (float DistanceToMatch; ShouldDistanceMatchStop() && (DistanceToMatch = GetPredictedStopDistance()) > 0.f)
	{
		UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluatorRef, DistanceToMatch, LocomotionDistanceCurveName);
	}
	else
	{
		USequenceEvaluatorLibrary::AdvanceTime(Context, SequenceEvaluatorRef, 1.f);
	}
}

float UZodiacAnimInstance::GetPredictedStopDistance() const
{
	if (UCharacterMovementComponent* CharMoveComp = GetMovementComponent())
	{
		const FVector StopLocation = UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(
		CharMoveComp->GetLastUpdateVelocity(),
		CharMoveComp->bUseSeparateBrakingFriction,
		CharMoveComp->BrakingFriction,
		CharMoveComp->GroundFriction,
		CharMoveComp->BrakingFrictionFactor,
		CharMoveComp->BrakingDecelerationWalking);

		const float PredictedStopDistance = UKismetMathLibrary::VSizeXY(StopLocation);
		return PredictedStopDistance;
	}

	return 0.0f;
}

void UZodiacAnimInstance::SetUpStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	const int Index = LocalVelocityDirection;
	UAnimSequence* ADSAnim = WalkStartCardinals.GetAnimSequences()[Index];
	UAnimSequence* JogAnim = JogStartCardinals.GetAnimSequences()[Index];
	UAnimSequence* SprintAnim = SprintStartCardinals.GetAnimSequences()[Index];
	
	UAnimSequence* SequenceToPlay = GameplayTag_IsSprinting ? SprintAnim : GameplayTag_IsADS ? ADSAnim : JogAnim;

	EAnimNodeReferenceConversionResult Result;
	FSequenceEvaluatorReference SequenceEvaluatorRef = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	SequenceEvaluatorRef = USequenceEvaluatorLibrary::SetSequence(SequenceEvaluatorRef, SequenceToPlay);

	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluatorRef, 0.f);
	StrideWarpingStartAlpha = 0.f;
}

void UZodiacAnimInstance::UpdateStartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	const float ExplicitTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEvaluatorRef);
	const float Alpha = ExplicitTime - StrideWarpingBlendInStartOffset;

	const FVector2D InRange = FVector2D(0.f, StrideWarpingBlendInDurationScaled);
	const FVector2D OutRange = FVector2D(0.f, 1.f);
	StrideWarpingStartAlpha = FMath::GetMappedRangeValueClamped(InRange, OutRange, Alpha);

	const float ClampMin = FMath::Lerp(StrideWarpingBlendInDurationScaled, PlayRateClampStartsPivots.X, StrideWarpingStartAlpha);
	const float ClampMax = PlayRateClampStartsPivots.Y;
	const FVector2D PlayRateClamp = FVector2D(ClampMin, ClampMax);

	UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(
		Context, SequenceEvaluatorRef, DisplacementSinceLastUpdate, LocomotionDistanceCurveName, PlayRateClamp);
}

void UZodiacAnimInstance::SetupPivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	PivotStartingAcceleration = LocalAcceleration2D;

	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	UAnimSequence* SequenceToPlay = GetDesiredPivotSequence(CardinalDirectionFromAcceleration);
	USequenceEvaluatorLibrary::SetSequence(SequenceEvaluatorRef, SequenceToPlay);
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluatorRef, 0.f);

	StrideWarpingPivotAlpha = 0.f;
	TimeAtPivotStop = 0.f;
	LastPivotTime = 0.2f;
}

void UZodiacAnimInstance::UpdatePivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	const float ExplicitTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEvaluatorRef);

	// allow switching the selected pivot for a short duration at the beginning
	if (LastPivotTime > 0.f)
	{
		UAnimSequence* NewDesiredAnimSequence = GetDesiredPivotSequence(CardinalDirectionFromAcceleration);
		const UAnimSequenceBase* CurrentSequence = USequenceEvaluatorLibrary::GetSequence(SequenceEvaluatorRef);
		if (NewDesiredAnimSequence != CurrentSequence)
		{
			USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEvaluatorRef, NewDesiredAnimSequence, 0.2f);
			PivotStartingAcceleration = LocalAcceleration2D;
		}
	}

	// does acceleration oppose velocity?
	const float DotProduct = FVector::DotProduct(LocalVelocity2D, LocalAcceleration2D);
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
			Context, SequenceEvaluatorRef, DisplacementSinceLastUpdate, LocomotionDistanceCurveName, PlayRateClamp);
	}
}

UAnimSequence* UZodiacAnimInstance::GetDesiredPivotSequence(EAnimEnum_CardinalDirection InDirection)
{
	UAnimSequence* JogAnim = JogPivotCardinals.GetAnimSequences()[InDirection];
	UAnimSequence* ADSAnim = WalkPivotCardinals.GetAnimSequences()[InDirection];
	UAnimSequence* SprintAnim = SprintPivotCardinals.GetAnimSequences()[InDirection];

	return GameplayTag_IsSprinting ? SprintAnim : GameplayTag_IsADS ? ADSAnim : JogAnim;
}

void UZodiacAnimInstance::UpdateCycleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	const int Index = LocalVelocityDirection;
	
	UAnimSequence* JogAnim = JogCardinals.GetAnimSequences()[Index];
	UAnimSequence* WalkAnim = WalkCardinals.GetAnimSequences()[Index];
	UAnimSequence* SprintAnim = SprintCardinals.GetAnimSequences()[Index];

	UAnimSequence* SequenceToPlay = GameplayTag_IsSprinting ? SprintAnim : GameplayTag_IsADS ? WalkAnim : JogAnim;
	
	EAnimNodeReferenceConversionResult Result;
	const float BlendTime = 1.0f;
	const FSequencePlayerReference SequencePlayerRef = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);
	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayerRef, SequenceToPlay, BlendTime);

	const float SpeedToMatch = DisplacementSpeed;
	// @TODO: currently it's hardcoded value of Murdock Jog Fwd animation marching speed.
	// const float SourceAnimSpeed = 260/0.73;
	// const float AdjustedPlayrate = SpeedToMatch / SourceAnimSpeed; 
	// USequencePlayerLibrary::SetPlayRate(SequencePlayerRef, AdjustedPlayrate);
	
	const float LerpTarget = IsRunningIntoWall ? 0.5f : 1.f;
	StrideWarpingCycleAlpha = FMath::FInterpTo(StrideWarpingCycleAlpha, LerpTarget, GetDeltaSeconds(), 10.f);
}

float UZodiacAnimInstance::GetOrientationAngle() const
{
	const float LeftDirectionAngle = -90.0f;
	const float RightDirectionAngle = 90.0f;
	const float BackwardDirectionAngle = 180.0f;
	
	switch (LocalVelocityDirection)
	{
	default:
	case Front:
		return LocalVelocityDirectionAngle;
	case Left:
		// -45 - (-90) = 45
		return LocalVelocityDirectionAngle - LeftDirectionAngle;
	case Right:
		return LocalVelocityDirectionAngle - RightDirectionAngle;
	case Backward:
		if (LocalVelocityDirectionAngle > 0)
		{
			// 135(backward & right) - 180(backward) = -45 (orient left) 
			return LocalVelocityDirectionAngle - BackwardDirectionAngle;
		}
		else
		{
			// 180(backward) + -135(backward & left) = 45 (orient right)
			return BackwardDirectionAngle + LocalVelocityDirectionAngle; 
		}
	}
}

void UZodiacAnimInstance::SetUpTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	TurnInPlaceAnimTime = 0.f;

	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluatorRef, 0.f);
}

void UZodiacAnimInstance::UpdateTurnInPlaceAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequenceEvaluatorReference SequenceEvaluatorRef = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, Result);

	UAnimSequence* SequenceToPlay = SelectTurnInPlaceAnimation(TurnInPlaceRotationDirection);
	USequenceEvaluatorLibrary::SetSequenceWithInertialBlending(Context, SequenceEvaluatorRef, SequenceToPlay, 0.2f);

	TurnInPlaceAnimTime += GetDeltaSeconds();
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluatorRef, TurnInPlaceAnimTime);
}

void UZodiacAnimInstance::SetUpTurnInPlaceRotationState(const FAnimUpdateContext& Context,
	const FAnimNodeReference& Node)
{
	TurnInPlaceRotationDirection = -FMath::Sign(RootYawOffset);
}

void UZodiacAnimInstance::SetUpTurnInPlaceRecoveryState(const FAnimUpdateContext& Context,
	const FAnimNodeReference& Node)
{
	TurnInPlaceRecoveryDirection = TurnInPlaceRotationDirection;
}

void UZodiacAnimInstance::UpdateTurnInPlaceRecoveryAnim(const FAnimUpdateContext& Context,
	const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	const FSequencePlayerReference SequencePlayerRef = USequencePlayerLibrary::ConvertToSequencePlayer(Node, Result);

	UAnimSequence* SequenceToPlay = SelectTurnInPlaceAnimation(TurnInPlaceRecoveryDirection);
	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayerRef, SequenceToPlay, 0.2f);
}

UAnimSequence* UZodiacAnimInstance::SelectTurnInPlaceAnimation(float Direction) const
{
	UAnimSequence* TurnLeft = TurnInPlaceLeft;
	UAnimSequence* TurnRight = TurnInPlaceRight;

	return (Direction > 0.f) ? TurnRight : TurnLeft;
}
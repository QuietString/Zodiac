// the.quiet.string@gmail.com

#include "ZodiacAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AnimationStateMachineLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Character/ZodiacPlayerCharacter.h"


UZodiacAnimInstance::UZodiacAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZodiacAnimInstance::InitializePropertyMap()
{
	if (APawn* Pawn = TryGetPawnOwner())
	{
		if (AZodiacPlayerCharacter* Character = CastChecked<AZodiacPlayerCharacter>(Pawn))
		{
			if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
			{
				GameplayTagPropertyMap.Initialize(this, ASC);
			}
		}
	}
}

float UZodiacAnimInstance::GetGroundDistance() const
{
	return GroundDistance;
}

void UZodiacAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UZodiacAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const AZodiacPlayerCharacter* Character = Cast<AZodiacPlayerCharacter>(GetOwningActor());
	if (!Character)
	{
		return;
	}

	UZodiacCharacterMovementComponent* CharMoveComp = CastChecked<UZodiacCharacterMovementComponent>(Character->GetCharacterMovement());
	const FZodiacCharacterGroundInfo& GroundInfo = CharMoveComp->GetGroundInfo();
	GroundDistance = GroundInfo.GroundDistance;
}

void UZodiacAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if (!TryGetPawnOwner() || !GetMovementComponent())
	{
		return;
	}

	UpdateLocationData(DeltaSeconds);
	UpdateRotationData(DeltaSeconds);

	// ignore first tick
	if (IsFirstUpdate)
	{
		DisplacementSinceLastUpdate = 0.f;
		DisplacementSpeed = 0.f;
		YawDeltaSinceLastUpdate = 0.f;
		AdditiveLeanAngle = 0.f;
	}

	UpdateVelocityData();
	UpdateAccelerationData();
	UpdateWallDetectionHeuristic();
	UpdateCharacterStateData(DeltaSeconds);
	UpdateBlendWeightData(DeltaSeconds);
	UpdateRootYawOffset(DeltaSeconds);
	UpdateAimingData();
	UpdateJumpFallData();

	DisableControlRig = GetCurveValue(FName("DisableLegIK")) > 0.f;

	IsFirstUpdate = false;
}

UCharacterMovementComponent* UZodiacAnimInstance::GetMovementComponent() const
{
	if (APawn* Pawn = TryGetPawnOwner())
	{
		if (UCharacterMovementComponent* CharMoveComp = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent()))
		{
			return CharMoveComp;
		}
	}

	return nullptr;
}

AZodiacPlayerCharacter* UZodiacAnimInstance::GetZodiacPlayerCharacter() const
{
	if (APawn* Pawn = TryGetPawnOwner())
	{
		if (AZodiacPlayerCharacter* ZodiacPlayerCharacter = Cast<AZodiacPlayerCharacter>(Pawn))
		{
			return ZodiacPlayerCharacter;
		}
	}

	return nullptr;
}

void UZodiacAnimInstance::UpdateLocationData(const float DeltaSeconds)
{
	const FVector PositionDiff = GetOwningActor()->GetActorLocation() - WorldLocation;
	DisplacementSinceLastUpdate = UKismetMathLibrary::VSizeXY(PositionDiff);

	WorldLocation = GetOwningActor()->GetActorLocation();
	
	DisplacementSpeed = UKismetMathLibrary::SafeDivide(DisplacementSinceLastUpdate, DeltaSeconds);
}

void UZodiacAnimInstance::UpdateRotationData(const float DeltaSeconds)
{
	const FRotator OldWorldRotation = WorldRotation;
	WorldRotation = GetOwningActor()->GetActorRotation();
	
	const FRotator RotationDiff = WorldRotation - OldWorldRotation;
	YawDeltaSinceLastUpdate = FMath::UnwindDegrees(RotationDiff.Yaw);
	YawDeltaSpeed = UKismetMathLibrary::SafeDivide(YawDeltaSinceLastUpdate, DeltaSeconds);

	const float LeanWeight = GameplayTag_IsADS ? 0.025f : 0.0375;
	AdditiveLeanAngle = YawDeltaSpeed * LeanWeight;
}

void UZodiacAnimInstance::UpdateVelocityData()
{
	const bool WasMovingLastUpdate = !(LocalVelocity2D.IsZero());

	WorldVelocity = TryGetPawnOwner()->GetVelocity();
	const FVector WorldVelocity2D = FVector(WorldVelocity.X, WorldVelocity.Y, 0.f);

	LocalVelocity2D = WorldRotation.UnrotateVector(WorldVelocity2D);

	LocalVelocityDirectionAngle = UKismetAnimationLibrary::CalculateDirection(WorldVelocity2D, WorldRotation);
	LocalVelocityDirectionAngleWithOffset = LocalVelocityDirectionAngle - RootYawOffset;

	LocalVelocityDirection = SelectCardinalDirectionFromAngle(
		LocalVelocityDirectionAngleWithOffset, CardinalDirectionDeadZone, LocalVelocityDirection, WasMovingLastUpdate);

	const float VelocitySquared = UKismetMathLibrary::VSizeXYSquared(LocalVelocity2D);
	HasVelocity = !(UKismetMathLibrary::NearlyEqual_FloatFloat(VelocitySquared, 0.f, 0.000001));
}

EAnimEnum_CardinalDirection UZodiacAnimInstance::GetOppositeCardinalDirection(
	const EAnimEnum_CardinalDirection CurrentDirection)
{
	switch (CurrentDirection)
	{
	case EAnimEnum_CardinalDirection::Front:
		return EAnimEnum_CardinalDirection::Backward;

	case EAnimEnum_CardinalDirection::Backward:
		return EAnimEnum_CardinalDirection::Front;

	case EAnimEnum_CardinalDirection::Left:
		return EAnimEnum_CardinalDirection::Right;

	case EAnimEnum_CardinalDirection::Right:
		return EAnimEnum_CardinalDirection::Left;

	default: return CurrentDirection;
	}
}

void UZodiacAnimInstance::UpdateAccelerationData()
{
	FVector WorldAcceleration2D = GetMovementComponent()->GetCurrentAcceleration();
	WorldAcceleration2D.Z = 0;
	LocalAcceleration2D = WorldRotation.UnrotateVector(WorldAcceleration2D);

	const float AccelerationSquared = UKismetMathLibrary::VSizeXYSquared(LocalAcceleration2D);
	HasAcceleration = !(UKismetMathLibrary::NearlyEqual_FloatFloat(AccelerationSquared, 0.0, 0.000001));
	PivotDirection2D = UKismetMathLibrary::VLerp(PivotDirection2D, WorldAcceleration2D.GetSafeNormal(0.0001), 0.5);
	PivotDirection2D.Normalize(0.0001);
	const float AccelerationDirection = UKismetAnimationLibrary::CalculateDirection(PivotDirection2D, WorldRotation);

	const EAnimEnum_CardinalDirection CardinalDirection = SelectCardinalDirectionFromAngle(
		AccelerationDirection, CardinalDirectionDeadZone, EAnimEnum_CardinalDirection::Front, false);

	CardinalDirectionFromAcceleration = GetOppositeCardinalDirection(CardinalDirection);
}

void UZodiacAnimInstance::UpdateCharacterStateData(const float DeltaSeconds)
{
	IsOnGround = GetMovementComponent()->IsMovingOnGround();
	
	ADSStateChanged = (GameplayTag_IsADS != WasADSLastUpdate) ? true : false;
	WasADSLastUpdate = GameplayTag_IsADS;

	GameplayTag_IsSprinting = GetZodiacPlayerCharacter() ? GetZodiacPlayerCharacter()->bIsSprinting : false;
	SprintStateChanged = ( GameplayTag_IsSprinting != bWasSprintingLastUpdate );
	bWasSprintingLastUpdate = GameplayTag_IsSprinting;
	
	TimeSinceFiredWeapon = GameplayTag_IsFiring ? 0.f : TimeSinceFiredWeapon + DeltaSeconds;

	IsJumping = false;
	IsFalling = false;
	if (GetMovementComponent()->MovementMode == MOVE_Falling)
	{
		WorldVelocity.Z > 0.f ? IsJumping = true : IsFalling = true;
	}
}

void UZodiacAnimInstance::UpdateBlendWeightData(float DeltaSeconds)
{
	const float NewAdditiveWeight = UKismetMathLibrary::FInterpTo(UpperBodyDynamicAdditiveWeight, 0.f, DeltaSeconds, 6.f);

	UpperBodyDynamicAdditiveWeight = IsAnyMontagePlaying() && IsOnGround ? 1.f : NewAdditiveWeight;

}

void UZodiacAnimInstance::ProcessTurnYawCurve()
{
	float PreviousTurnYawCurveValue = TurnYawCurveValue;

	const float TurnYawWeight = GetCurveValue(FName("TurnYawWeight"));
	const bool IsWeightSmall = UKismetMathLibrary::NearlyEqual_FloatFloat(TurnYawWeight, 0.f, 0.0001);

	if (IsWeightSmall)
	{
		TurnYawCurveValue = 0.f;
		PreviousTurnYawCurveValue = 0.f;
	}
	else
	{
		const float RemainingTurnYaw = GetCurveValue(FName("RemainingTurnYaw"));
		TurnYawCurveValue = RemainingTurnYaw / TurnYawWeight;

		if (PreviousTurnYawCurveValue != 0.f)
		{
			const float NewRootYawOffset = RootYawOffset - (TurnYawCurveValue - PreviousTurnYawCurveValue);
			SetRootYawOffset(NewRootYawOffset);
		}
	}
}

void UZodiacAnimInstance::SetRootYawOffset(const float InRootYawOffset)
{
	const float RootYawOffsetNormalized = UKismetMathLibrary::NormalizeAxis(InRootYawOffset);
	const FVector2D OffsetClamp = RootYawOffsetAngleClamp;

	const float ClampedOffset = UKismetMathLibrary::ClampAngle(RootYawOffsetNormalized, OffsetClamp.X, OffsetClamp.Y);
	RootYawOffset = (OffsetClamp.X == OffsetClamp.Y) ? RootYawOffsetNormalized : ClampedOffset;

	//UE_LOG(LogTemp, Warning, TEXT("aimyaw: %f"), AimYaw);

	AimYaw = -RootYawOffset;
}

void UZodiacAnimInstance::UpdateRootYawOffset(const float DeltaSeconds)
{
	if (RootYawOffsetMode == EAnimEnum_RootYawOffsetMode::Accumulate)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Accumulate, RootYawOffset: %f, SinceLast: %f"), RootYawOffset, YawDeltaSinceLastUpdate);
		SetRootYawOffset(RootYawOffset - YawDeltaSinceLastUpdate);
	}

	if (RootYawOffsetMode == EAnimEnum_RootYawOffsetMode::BlendOut || GameplayTag_IsDashing)
	{
		const float OffsetSpringInterp = UKismetMathLibrary::FloatSpringInterp(
			RootYawOffset, 0.f, RootYawOffsetSpringState, 80.f,
			1.f, DeltaSeconds, 1.f, 0.5f);

		SetRootYawOffset(OffsetSpringInterp);
	}

	RootYawOffsetMode = EAnimEnum_RootYawOffsetMode::BlendOut;
}

bool UZodiacAnimInstance::IsMovingPerpendicularToInitialPivot() const
{
	const bool IsPivotHorizontal = (PivotInitialDirection == EAnimEnum_CardinalDirection::Front ||
		PivotInitialDirection == EAnimEnum_CardinalDirection::Backward);

	const bool IsVelocityHorizontal = (LocalVelocityDirection == EAnimEnum_CardinalDirection::Front ||
		LocalVelocityDirection == EAnimEnum_CardinalDirection::Backward);

	const bool IsPivotVertical = (PivotInitialDirection == EAnimEnum_CardinalDirection::Right ||
		PivotInitialDirection == EAnimEnum_CardinalDirection::Left);

	const bool IsVelocityVertical = (LocalVelocityDirection == EAnimEnum_CardinalDirection::Right ||
		LocalVelocityDirection == EAnimEnum_CardinalDirection::Left);

	const bool IsPerpendicularToHorizontal = IsPivotHorizontal && !IsVelocityHorizontal;
	const bool IsPerpendicularToVertical = IsPivotVertical && !IsVelocityVertical;

	return IsPerpendicularToHorizontal || IsPerpendicularToVertical;
}

void UZodiacAnimInstance::UpdateAimingData()
{
	AimPitch = UKismetMathLibrary::NormalizeAxis(TryGetPawnOwner()->GetBaseAimRotation().Pitch);
}

void UZodiacAnimInstance::UpdateIdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference AnimationStateResultReference;
	EAnimNodeReferenceConversionResult Result;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationStateResultReference, Result);
	bool IsStateBlendingOut = UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationStateResultReference);

	if (IsStateBlendingOut)
	{
		TurnYawCurveValue = 0.f;
	}
	else
	{
		RootYawOffsetMode = EAnimEnum_RootYawOffsetMode::Accumulate;
		ProcessTurnYawCurve();
	}
}

void UZodiacAnimInstance::SetUpStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	StartDirection = LocalVelocityDirection;
}

void UZodiacAnimInstance::UpdateStartState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference AnimationStateResultReference;
	EAnimNodeReferenceConversionResult Result;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationStateResultReference, Result);
	const bool IsBlendingOut = UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationStateResultReference);

	if (!IsBlendingOut) RootYawOffsetMode = EAnimEnum_RootYawOffsetMode::Hold;
}

void UZodiacAnimInstance::UpdateStopState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	FAnimationStateResultReference AnimationStateResultReference;
	EAnimNodeReferenceConversionResult Result;
	UAnimationStateMachineLibrary::ConvertToAnimationStateResult(Node, AnimationStateResultReference, Result);
	const bool IsBlendingOut = UAnimationStateMachineLibrary::IsStateBlendingOut(Context, AnimationStateResultReference);

	if (!IsBlendingOut) RootYawOffsetMode = EAnimEnum_RootYawOffsetMode::Accumulate;
}

void UZodiacAnimInstance::SetupPivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	PivotInitialDirection = LocalVelocityDirection;
}

void UZodiacAnimInstance::UpdatePivotState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (LastPivotTime > 0.f)
	{
		LastPivotTime = LastPivotTime - Context.GetContext()->GetDeltaTime();
	}
}

void UZodiacAnimInstance::UpdateLocomotionStateMachine(const FAnimUpdateContext& Context,
	const FAnimNodeReference& Node)
{
	UAnimInstance* LinkedAnimLayer = GetLinkedAnimGraphInstanceByTag(FName("StartLayerNode"));

	if (!IsFirstUpdate)
	{
		LinkedLayerChanged = UKismetMathLibrary::NotEqual_ObjectObject(LinkedAnimLayer, LastLinkedLayer);
	}

	LastLinkedLayer = LinkedAnimLayer;
}

bool UZodiacAnimInstance::IdleToStart() const
{
	return HasAcceleration || (GameplayTag_IsMelee && HasVelocity);
}

bool UZodiacAnimInstance::StartToCycle3()
{
	const FName StateMachineName = TEXT("LocomotionSM");
	const int32 MachineIndex = GetStateMachineIndex(StateMachineName);

	const bool Condition1 = StartDirection != LocalVelocityDirection;
	const bool Condition3 = ADSStateChanged;
	const bool Condition4 =( GetInstanceCurrentStateElapsedTime(MachineIndex) > 0.15f) && (DisplacementSpeed < 10.f);

	return (Condition1 || Condition3 || Condition4);
}

bool UZodiacAnimInstance::StartToCycle() const
{
	// Transition to Cycle early if the offset is too large.
	// E.g. if the camera is on the right side of the character and the user presses forward,
	// we don't want to strafe left for a long period of time, we want to jog forward.
	// This transition uses the SyncGroupNameToRequireValidMarkersRule setting
	// to ensure we wait until there are valid sync markers so we smoothly enter Cycle.
	return FMath::Abs(RootYawOffset) > 60.f;
}

bool UZodiacAnimInstance::StopToIdle() const
{
	return ADSStateChanged;
}

bool UZodiacAnimInstance::StopRule() const
{
	return !(HasAcceleration || (GameplayTag_IsMelee && HasVelocity));
}

bool UZodiacAnimInstance::TransitionRule_PivotToCycle() const
{
	return (ADSStateChanged || (IsMovingPerpendicularToInitialPivot() && LastPivotTime <= 0.f));
}

bool UZodiacAnimInstance::TransitionRule_PivotSourcesToPivot() const
{
	// Check if velocity (where we're moving) is opposite to acceleration (where we want to be moving).
	return ((LocalVelocity2D.Dot(LocalAcceleration2D) < 0.f) && !IsRunningIntoWall);
}

bool UZodiacAnimInstance::JumpStartLoopToJumpApex() const
{
	return TimeToJumpApex < 0.4f;
}

bool UZodiacAnimInstance::FallLoopToFallLand() const
{
	return GroundDistance < 200.f;
}

void UZodiacAnimInstance::UpdateWallDetectionHeuristic()
{
	bool IsAccelerationLargeEnough = (UKismetMathLibrary::VSizeXY(LocalAcceleration2D) > 0.1f) ? true : false;
	bool IsVelocitySmall = (UKismetMathLibrary::VSizeXY(LocalVelocity2D) < 200.f) ? true : false;
	bool IsSameDirection;

	FVector AccelerationNorm = LocalAcceleration2D.GetSafeNormal(0.0001);
	FVector VelocityNorm = LocalVelocity2D.GetSafeNormal(0.0001);
	float AngleDiff = FVector::DotProduct(AccelerationNorm, VelocityNorm);

	IsSameDirection = UKismetMathLibrary::InRange_FloatFloat(AngleDiff, -0.6f, 0.6f, true, true);

	IsRunningIntoWall = (IsAccelerationLargeEnough && IsVelocitySmall && IsSameDirection) ? true : false;
}

void UZodiacAnimInstance::UpdateJumpFallData()
{
	float NewTimeToJumpApex = -WorldVelocity.Z / GetMovementComponent()->GetGravityZ();
	TimeToJumpApex = IsJumping ? NewTimeToJumpApex : 0.f;
}

EAnimEnum_CardinalDirection UZodiacAnimInstance::SelectCardinalDirectionFromAngle(float Angle, float DeadZone,
	EAnimEnum_CardinalDirection CurrentDirection, bool UseCurrentDirection)
{
	float AbsAngle = FMath::Abs(Angle);
	float FwdDeadZone = DeadZone;
	float BwdDeadZone = FwdDeadZone;

	if (UseCurrentDirection)
	{
		switch (CurrentDirection)
		{
		case Front:
			FwdDeadZone *= 2;
			break;

		case Backward:
			BwdDeadZone *= 2;
			break;

		default:
			break;
		}
	}

	if (AbsAngle <= FwdDeadZone + 45.f) return Front;

	if (AbsAngle >= 135.f - BwdDeadZone) return Backward;

	if (Angle > 0.f) return Right;

	return Left;
}

bool UZodiacAnimInstance::ShouldDistanceMatchStop() const
{
	// when movement doesn't exist, but a character is still moving.
	return HasVelocity && !HasAcceleration;
}

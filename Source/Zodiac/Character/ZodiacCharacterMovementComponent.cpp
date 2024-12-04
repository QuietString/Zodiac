// the.quiet.string@gmail.com

#include "ZodiacCharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "KismetAnimationLibrary.h"
#include "ZodiacGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacCharacterMovementComponent)

namespace ZodiacCharacter
{
	static float GroundTraceDistance = 100000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(TEXT("ZodiacCharacter.GroundTraceDistance"), GroundTraceDistance, TEXT("Distance to trace down when generating ground information."), ECVF_Cheat);
};


UZodiacCharacterMovementComponent::UZodiacCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

namespace PackedMovementModeConstants
{
	const uint32 GroundShift = FMath::CeilLogTwo(MOVE_MAX); // 3
	const uint8 CustomModeThr = 2 * (1 << GroundShift); //  2 * 8 = 16 = 0b10000
	const uint8 GroundMask = (1 << GroundShift) - 1; // 8 - 1 = 7 0b111
	const uint8 MainMask = 15 << 4; // 11110000x
}

uint8 UZodiacCharacterMovementComponent::PackNetworkMovementMode() const
{
	const uint8 GroundModeBit = (GetGroundMovementMode() == MOVE_Walking ? 0 : 1);
	uint8 MainMovementMode = uint8(MovementMode.GetValue()) | (GroundModeBit << PackedMovementModeConstants::GroundShift);

	return MainMovementMode | (CustomMovementMode << 4);
}

void UZodiacCharacterMovementComponent::UnpackNetworkMovementMode(const uint8 ReceivedMode, TEnumAsByte<EMovementMode>& OutMode, uint8& OutCustomMode,
	TEnumAsByte<EMovementMode>& OutGroundMode) const
{
	OutMode = TEnumAsByte<EMovementMode>(ReceivedMode & PackedMovementModeConstants::GroundMask);
	OutCustomMode = (ReceivedMode & PackedMovementModeConstants::MainMask) >> 4;
	const uint8 GroundModeBit = (ReceivedMode >> PackedMovementModeConstants::GroundShift);
	OutGroundMode = TEnumAsByte<EMovementMode>(GroundModeBit == 0 ? MOVE_Walking : MOVE_NavWalking);
}

void UZodiacCharacterMovementComponent::SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode)
{
	// Almost same as UCharacterMovementComponent's implementation but
	// 1. removed checking (NewMovementMode == MOVE_CUSTOM)
	// 2. allowing custom mode even when MovementMode is not MOVE_CUSTOM 
	
	// If trying to use NavWalking but there is no navmesh, use walking instead.
	if (NewMovementMode == MOVE_NavWalking)
	{
		if (GetNavData() == nullptr)
		{
			NewMovementMode = MOVE_Walking;
		}
	}

	// Do nothing if nothing is changing.
	if (MovementMode == NewMovementMode && CustomMovementMode == NewCustomMode)
	{
		return;
	}

	const EMovementMode PrevMovementMode = MovementMode;
	const uint8 PrevCustomMode = CustomMovementMode;
	
	CustomMovementMode = NewCustomMode;
	MovementMode = NewMovementMode;

	// We allow setting movement mode before we have a component to update, in case this happens at startup.
	if (!HasValidData())
	{
		return;
	}
	
	// Handle change in movement mode
	OnMovementModeChanged(PrevMovementMode, PrevCustomMode);
	
	bMovementModeDirty = true; // lets async callback know movement mode was dirtied on game thread
}

void UZodiacCharacterMovementComponent::SimulateMovement(float DeltaTime)
{
	if (bHasReplicatedAcceleration)
	{
		// Preserve our replicated acceleration
		const FVector OriginalAcceleration = Acceleration;
		Super::SimulateMovement(DeltaTime);
		Acceleration = OriginalAcceleration;
	}
	else
	{
		Super::SimulateMovement(DeltaTime);
	}
}

void UZodiacCharacterMovementComponent::JumpOff(AActor* MovementBaseActor)
{
	// Don't set MovementMode to MOVE_Falling when CustomMovementMode is MOVE_Traversal.
	if ( !bPerformingJumpOff )
	{
		bPerformingJumpOff = true;
		if ( CharacterOwner )
		{
			const float MaxSpeed = GetMaxSpeed() * 0.85f;
			Velocity += MaxSpeed * GetBestDirectionOffActor(MovementBaseActor);
			if ( Velocity.Size2D() > MaxSpeed )
			{
				Velocity = MaxSpeed * Velocity.GetSafeNormal();
			}

			if (HasCustomGravity())
			{
				FVector GravityRelativeVelocity = RotateWorldToGravity(Velocity);
				GravityRelativeVelocity.Z = JumpOffJumpZFactor * JumpZVelocity;
				Velocity = RotateGravityToWorld(GravityRelativeVelocity);
			}
			else
			{
				Velocity.Z = JumpOffJumpZFactor * JumpZVelocity;
			}

			if (CustomMovementMode != Move_Custom_Traversal)
			{
				SetMovementMode(MOVE_Falling);	
			}
		}
		bPerformingJumpOff = false;
	}
}

float UZodiacCharacterMovementComponent::GetMaxSpeed() const
{
	if (MovementMode == MOVE_Walking)
	{
		return CalculateMaxSpeed();
	}

	return Super::GetMaxSpeed();
}

bool UZodiacCharacterMovementComponent::CanAttemptJump() const
{
	// Same as UCharacterMovementComponent's implementation but without the crouch check
	return IsJumpAllowed() && (IsMovingOnGround() || IsFalling()); // Falling included for double-jump and non-zero jump hold time, but validated by character.
}

void UZodiacCharacterMovementComponent::SetDefaultMovementMode()
{
	// Same as UCharacterMovementComponent's implementation but with DefaultCustomMovementMode

	// check for water volume
	if (CanEverSwim() && IsInWater())
	{
		SetMovementMode(DefaultWaterMovementMode);
	}
	else if ( !CharacterOwner || MovementMode != DefaultLandMovementMode || CustomMovementMode != DefaultCustomMovementMode)
	{
		const float SavedVelocityZ = Velocity.Z;
		SetMovementMode(DefaultLandMovementMode, DefaultCustomMovementMode);

		// Avoid 1-frame delay if trying to walk but walking fails at this location.
		if (MovementMode == MOVE_Walking && GetMovementBase() == NULL)
		{
			Velocity.Z = SavedVelocityZ; // Prevent temporary walking state from zeroing Z velocity.
			SetMovementMode(MOVE_Falling);
		}
	}
}

void UZodiacCharacterMovementComponent::SetPostLandedPhysics(const FHitResult& Hit)
{
	// Same as UCharacterMovementComponent's implementation, but with DefaultCustomMovementMode
	
	if( CharacterOwner )
	{
		if (CanEverSwim() && IsInWater())
		{
			SetMovementMode(MOVE_Swimming);
		}
		else
		{
			const FVector PreImpactAccel = Acceleration + (IsFalling() ? -GetGravityDirection() * GetGravityZ() : FVector::ZeroVector);
			const FVector PreImpactVelocity = Velocity;

			if (DefaultLandMovementMode == MOVE_Walking ||
				DefaultLandMovementMode == MOVE_NavWalking ||
				DefaultLandMovementMode == MOVE_Falling)
			{
				SetMovementMode(GetGroundMovementMode(), DefaultCustomMovementMode);
			}
			else
			{
				SetDefaultMovementMode();
			}
			
			ApplyImpactPhysicsForces(Hit, PreImpactAccel, PreImpactVelocity);
		}
	}
}

bool UZodiacCharacterMovementComponent::HandlePendingLaunch()
{
	// Same as UCharacterMovementComponent's implementation, but don't change movement mode when it's MOVE_Flying
	if (!PendingLaunchVelocity.IsZero() && HasValidData())
	{
		Velocity = PendingLaunchVelocity;
		if (MovementMode != MOVE_Flying)
		{
			SetMovementMode(MOVE_Falling);	
		}
		PendingLaunchVelocity = FVector::ZeroVector;
		bForceNextFloorCheck = true;
		return true;
	}

	return false;
}

const FZodiacCharacterGroundInfo& UZodiacCharacterMovementComponent::GetGroundInfo()
{
	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame))
	{
		return CachedGroundInfo;
	}

	if (MovementMode == MOVE_Walking)
	{
		CachedGroundInfo.GroundHitResult = CurrentFloor.HitResult;
		CachedGroundInfo.GroundDistance = 0.0f;
	}
	else
	{
		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
		const FVector TraceStart(GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - ZodiacCharacter::GroundTraceDistance - CapsuleHalfHeight));

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ZodiacCharacterMovementComponent_GetGroundInfo), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(QueryParams, ResponseParam);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);

		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = ZodiacCharacter::GroundTraceDistance;

		if (MovementMode == MOVE_NavWalking)
		{
			CachedGroundInfo.GroundDistance = 0.0f;
		}
		else if (HitResult.bBlockingHit)
		{
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
		}
	}

	CachedGroundInfo.LastUpdateFrame = GFrameCounter;

	return CachedGroundInfo;
}

void UZodiacCharacterMovementComponent::SetReplicatedAcceleration(const FVector& InAcceleration)
{
	bHasReplicatedAcceleration = true;
	Acceleration = InAcceleration;
}

FZodiacMovementInputDirections UZodiacCharacterMovementComponent::GetMovementInputDirection()
{
    FZodiacMovementInputDirections Directions;

    // Get the pawn owner
    if (!PawnOwner)
    {
        return Directions;
    }

    // Get the pawn's forward and right vectors
    FVector PawnForward = PawnOwner->GetActorForwardVector();
    FVector PawnRight = PawnOwner->GetActorRightVector();

    // Get the last input vector
    FVector LastInputVector = GetLastInputVector();

    // Check if there is any movement input
    if (LastInputVector.IsNearlyZero())
    {
        return Directions; // No input detected
    }

    // Normalize the input vector
    FVector InputDirection = LastInputVector.GetSafeNormal();

    // Calculate dot products
    float ForwardDot = FVector::DotProduct(PawnForward, InputDirection);
    float RightDot = FVector::DotProduct(PawnRight, InputDirection);

    // Threshold to filter out insignificant input
    const float Threshold = 0.1f;

    // Array to store directions and their magnitudes
    struct FDirectionValue
    {
        EZodiacMovementInputDirection Direction;
        float Value;
    };

    TArray<FDirectionValue> DirectionValues;

    // Determine forward/backward direction
    if (ForwardDot > Threshold)
    {
        DirectionValues.Add({ EZodiacMovementInputDirection::Forward, ForwardDot });
    }
    else if (ForwardDot < -Threshold)
    {
        DirectionValues.Add({ EZodiacMovementInputDirection::Backward, -ForwardDot });
    }

    // Determine right/left direction
    if (RightDot > Threshold)
    {
        DirectionValues.Add({ EZodiacMovementInputDirection::Right, RightDot });
    }
    else if (RightDot < -Threshold)
    {
        DirectionValues.Add({ EZodiacMovementInputDirection::Left, -RightDot });
    }

    // Sort directions by their magnitude
    DirectionValues.Sort([](const FDirectionValue& A, const FDirectionValue& B)
    {
        return A.Value > B.Value;
    });

    // Assign primary and secondary directions
    if (DirectionValues.Num() >= 1)
    {
        Directions.PrimaryDirection = DirectionValues[0].Direction;
    }
    if (DirectionValues.Num() >= 2)
    {
        Directions.SecondaryDirection = DirectionValues[1].Direction;
    }

    return Directions;
}

float UZodiacCharacterMovementComponent::CalculateMaxSpeed() const
{
	FRotator Rotation = GetPawnOwner() ? GetPawnOwner()->GetActorRotation() : FRotator();
	float MovementAngle = FMath::Abs(UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation));
	float StrafeMap = StrafeSpeedMapCurve ? StrafeSpeedMapCurve->GetFloatValue(MovementAngle) : 1.0f;
	
	FVector DesiredSpeedRange;

	switch (CustomMovementMode)
	{
	case Move_Custom_Running:
		DesiredSpeedRange = RunSpeeds;
		break;

	default:
		DesiredSpeedRange = WalkSpeeds;
		break;
	}

	// Min value is used when strafing or moving backward.
	float MaxSpeed = (StrafeMap < 1.0f) ? DesiredSpeedRange.X : DesiredSpeedRange.Y;
	float MinSpeed = (StrafeMap < 1.0f) ? DesiredSpeedRange.Y : DesiredSpeedRange.Z;
	return UKismetMathLibrary::MapRangeClamped(StrafeMap, 0.0f, 1.0f, MaxSpeed, MinSpeed);
}

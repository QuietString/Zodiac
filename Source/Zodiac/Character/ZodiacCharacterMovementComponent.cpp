// the.quiet.string@gmail.com

#include "ZodiacCharacterMovementComponent.h"

#include "KismetAnimationLibrary.h"
#include "ZodiacCharacter.h"
#include "ZodiacCharacterType.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacCharacterMovementComponent)

namespace ZodiacCharacter
{
	static float GroundTraceDistance = 100000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(TEXT("ZodiacCharacter.GroundTraceDistance"), GroundTraceDistance, TEXT("Distance to trace down when generating ground information."), ECVF_Cheat);
};

UZodiacCharacterMovementComponent::UZodiacCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	
	ExtendedMovementConfig.DefaultExtendedMovement = EZodiacExtendedMovementMode::Running;
	ExtendedMovementConfig.MovementSpeedsMap.Add(EZodiacExtendedMovementMode::Walking, FVector(200.f, 175.f, 150.f));
	ExtendedMovementConfig.MovementSpeedsMap.Add(EZodiacExtendedMovementMode::Running, FVector(500.f, 350.f, 300.f));
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

float UZodiacCharacterMovementComponent::GetMaxSpeed() const
{
	if (MovementMode == MOVE_Walking || MovementMode == MOVE_NavWalking)
	{
		return CalculateMaxSpeed();
	}

	return Super::GetMaxSpeed();
}

void UZodiacCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (CustomMovementMode == Move_Custom_Traversal)
	{
		PhysFlying(deltaTime, Iterations);
	}
	
	if (CharacterOwner)
	{
		CharacterOwner->K2_UpdateCustomMovement(deltaTime);
	}
}

bool UZodiacCharacterMovementComponent::HandlePendingLaunch()
{
	// Same as UCharacterMovementComponent's implementation, but don't change movement mode when it's MOVE_Flying
	// to keep flying during AirBoost with launch
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

void UZodiacCharacterMovementComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ExtendedMovementMode, COND_None);
}

void UZodiacCharacterMovementComponent::SetReplicatedAcceleration(const FVector& InAcceleration)
{
	bHasReplicatedAcceleration = true;
	Acceleration = InAcceleration;
}

void UZodiacCharacterMovementComponent::ToggleSprint(bool bShouldSprint)
{
	// Don't strafe when sprinting
	ToggleStrafe(!bShouldSprint);
		
	EZodiacExtendedMovementMode NewMode = bShouldSprint ? EZodiacExtendedMovementMode::Sprinting : ExtendedMovementConfig.DefaultExtendedMovement;
	SetExtendedMovementMode(NewMode);
}

void UZodiacCharacterMovementComponent::ToggleStrafe(bool bShouldStrafe)
{
	bUseControllerDesiredRotation = bShouldStrafe;
	bOrientRotationToMovement = !bShouldStrafe;
}

void UZodiacCharacterMovementComponent::SetExtendedMovementMode(EZodiacExtendedMovementMode InMode)
{
	EZodiacExtendedMovementMode PrevMode = ExtendedMovementMode;
	if (PrevMode != InMode)
	{
		ExtendedMovementMode = InMode;
		OnExtendedMovementModeChanged(PrevMode);
	}
}

void UZodiacCharacterMovementComponent::SetExtendedMovementConfig(const FZodiacExtendedMovementConfig& InConfig)
{
	ExtendedMovementConfig = InConfig;
	SetExtendedMovementMode(InConfig.DefaultExtendedMovement);
}

FZodiacMovementInputDirections UZodiacCharacterMovementComponent::GetMovementInputDirection(bool bUseExplicitInputVector, FVector InputVector) const
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
    FVector LastInputVector = bUseExplicitInputVector ? InputVector : GetLastInputVector();

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

void UZodiacCharacterMovementComponent::OnExtendedMovementModeChanged(EZodiacExtendedMovementMode PreviousMovementMode)
{
	if (AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(CharacterOwner))
	{
		ZodiacCharacter->OnExtendedMovementModeChanged(PreviousMovementMode);
	}
}

float UZodiacCharacterMovementComponent::CalculateMaxSpeed() const
{
	FRotator Rotation = GetPawnOwner() ? GetPawnOwner()->GetActorRotation() : FRotator();
	MovementAngle = FMath::Abs(UKismetAnimationLibrary::CalculateDirection(Velocity, Rotation));
	
	float StrafeMap = StrafeSpeedMapCurve ? StrafeSpeedMapCurve->GetFloatValue(MovementAngle) : 0.f;

	// Use forward speed when not strafing.
	if (bUseControllerDesiredRotation)
	{
		StrafeMap = 0.f;
	}

	const FVector* FindVector = ExtendedMovementConfig.MovementSpeedsMap.Find(ExtendedMovementMode);
	
	FVector DesiredSpeedRange = FindVector ? *FindVector : FVector();

	// Min value is used when strafing or moving backward.
	float MaxSpeed = (StrafeMap < 1.0f) ? DesiredSpeedRange.X : DesiredSpeedRange.Y;
	float MinSpeed = (StrafeMap < 1.0f) ? DesiredSpeedRange.Y : DesiredSpeedRange.Z;
	return UKismetMathLibrary::MapRangeClamped(StrafeMap, 0.0f, 1.0f, MaxSpeed, MinSpeed);
}
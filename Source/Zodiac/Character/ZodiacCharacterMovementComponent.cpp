// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacCharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacCharacterMovementComponent)

namespace ZodiacCharacter
{
	static float GroundTraceDistance = 100000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(TEXT("ZodiacCharacter.GroundTraceDistance"), GroundTraceDistance, TEXT("Distance to trace down when generating ground information."), ECVF_Cheat);
};


UZodiacCharacterMovementComponent::UZodiacCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxSprintSpeed = 800;
	MaxWalkSpeedWithAiming = 300.0f;
}

namespace PackedMovementModeConstants
{
	const uint32 GroundShift = FMath::CeilLogTwo(MOVE_MAX); // 3
	const uint8 CustomModeThr = 2 * (1 << GroundShift); // 1111x
	const uint8 GroundMask = (1 << GroundShift) - 1; // 0111x
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
	// Same as UCharacterMovementComponent's implementation but
	// 1. without NewMovementMode == MOVE_CUSTOM check
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
	if (MovementMode == NewMovementMode && NewCustomMode == CustomMovementMode)
	{
		return;
	}

	const EMovementMode PrevMovementMode = MovementMode;
	const uint8 PrevCustomMode = CustomMovementMode;

	MovementMode = NewMovementMode;
	CustomMovementMode = NewCustomMode;

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

			if (CustomMovementMode != MOVE_Traversal)
			{
				SetMovementMode(MOVE_Falling);	
			}
		}
		bPerformingJumpOff = false;
	}
}

float UZodiacCharacterMovementComponent::GetMaxSpeed() const
{
	if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(CharacterOwner))
	{
		if (TagInterface->HasMatchingGameplayTag(ZodiacGameplayTags::Status_Stun))
		{
			return 0.0f;
		}		
	}

	if (MovementMode == MOVE_Walking)
	{
		switch (CustomMovementMode)
		{
		case MOVE_None:
			return MaxWalkSpeed;
		case MOVE_Aiming:
			return MaxWalkSpeedWithAiming;
		case MOVE_Sprinting:
			return MaxSprintSpeed;
		default:
			return MaxWalkSpeed;
		}
	}
	
	return Super::GetMaxSpeed();
}

bool UZodiacCharacterMovementComponent::CanAttemptJump() const
{
	// Same as UCharacterMovementComponent's implementation but without the crouch check
	return IsJumpAllowed() &&
		(IsMovingOnGround() || IsFalling()); // Falling included for double-jump and non-zero jump hold time, but validated by character.
}

void UZodiacCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
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
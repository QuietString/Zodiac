// the.quiet.string@gmail.com


#include "ZodiacHostAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "MaterialHLSLTree.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHeroAnimInstance.h"
#include "ZodiacLogChannels.h"
#include "Animation/AnimNodeReference.h"
#include "Character/ZodiacCharacter.h"
#include "Character/ZodiacHostCharacter.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "Character/ZodiacHeroCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "PoseSearch/MotionMatchingAnimNodeLibrary.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchResult.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHostAnimInstance)

void UZodiacHostAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* InASC)
{
	check(InASC);

	GameplayTagPropertyMap.Initialize(this, InASC);
}

void UZodiacHostAnimInstance::NativeInitializeAnimation()
{
	if (AZodiacCharacter* PawnOwner = Cast<AZodiacCharacter>(TryGetPawnOwner()))
	{
		OwningCharacter = PawnOwner;
		OwningCharacter->CallOrRegister_OnAbilitySystemInitialized(FOnAbilitySystemComponentInitialized::FDelegate::CreateUObject(this, &ThisClass::InitializeWithAbilitySystem));
	}

	MaxRootRotationOffset = MaxRootRotationOffset_Idle;
}

void UZodiacHostAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (AZodiacCharacter* PawnOwner = Cast<AZodiacCharacter>(TryGetPawnOwner()))
	{
		OwningCharacter = PawnOwner;
		
		if (UZodiacCharacterMovementComponent* CharacterMovement = Cast<UZodiacCharacterMovementComponent>(OwningCharacter->GetCharacterMovement()))
		{
			ZodiacCharMovComp = CharacterMovement;	
		}
	}
}

void UZodiacHostAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if (OwningCharacter && ZodiacCharMovComp)
	{
		UpdateLocationData(DeltaSeconds);
		UpdateRotationData();
		UpdateVelocityData();
		UpdateAccelerationData(DeltaSeconds);
		UpdateMovementData(DeltaSeconds);
		UpdateAimingData();
		UpdateGait();
		UpdateHeroData();
		
		if (CurrentSelectedDatabase.IsValid())
		{
			CurrentDatabaseTags = CurrentSelectedDatabase->Tags;
		}
	}
}

void UZodiacHostAnimInstance::UpdateLocationData(float DeltaSeconds)
{
	CharacterTransform = OwningCharacter->GetActorTransform();
	
	const FVector PositionDiff = OwningCharacter->GetActorLocation() - WorldLocation;
	DisplacementSinceLastUpdate = UKismetMathLibrary::VSizeXY(PositionDiff);

	WorldLocation = OwningCharacter->GetActorLocation();
	
	DisplacementSpeed = UKismetMathLibrary::SafeDivide(DisplacementSinceLastUpdate, DeltaSeconds);
}

void UZodiacHostAnimInstance::UpdateRotationData()
{
	WorldRotation = OwningCharacter->GetActorRotation();
}

void UZodiacHostAnimInstance::UpdateMovementData(float DeltaSeconds)
{
	ExtendedMovementMode = ZodiacCharMovComp->GetExtendedMovementMode();
	MovementAngle = ZodiacCharMovComp->GetMovementAngle();
	
	bIsTraversal = ZodiacCharMovComp->CustomMovementMode == Move_Custom_Traversal;

	bool IsAccelerationLargeEnough = UKismetMathLibrary::VSizeXY(LocalAcceleration2D) > 0.1f;
	bool IsVelocitySmall = UKismetMathLibrary::VSizeXY(LocalVelocity2D) < 200.f;

	FVector AccelerationNorm = LocalAcceleration2D.GetSafeNormal();
	FVector VelocityNorm = LocalVelocity2D.GetSafeNormal();
	float AngleDiff = AccelerationNorm | VelocityNorm;
	bool IsSameDirection = UKismetMathLibrary::InRange_FloatFloat(AngleDiff, -0.6f, 0.6f, true, true);

	bIsRunningIntoWall = IsAccelerationLargeEnough && IsVelocitySmall && IsSameDirection;
	bIsMoving = (!Velocity.Equals(FVector(0, 0, 0), 0.1) && !FutureVelocity.Equals(FVector(0, 0, 0), 0.1) && !bIsRunningIntoWall);
	bIsStrafing = ZodiacCharMovComp->GetIsStrafing();
	
	float TargetMaxOffset;
	
	if (ExtendedMovementMode == EZodiacExtendedMovementMode::Sprinting)
	{
		TargetMaxOffset = MaxRootRotationOffset_Sprint;
	}
	else if (bIsFocus || bIsADS)
	{
		TargetMaxOffset = MaxRootRotationOffset_Focus;
	}
	else
	{
		TargetMaxOffset = MaxRootRotationOffset_Idle;
	}

	MaxRootRotationOffset = FMath::FInterpTo(MaxRootRotationOffset, TargetMaxOffset, DeltaSeconds, InterpSpeed_MaxRootRotationOffset);
}

void UZodiacHostAnimInstance::UpdateAimingData()
{
	AimYaw_Last = AimYaw;
	
	FRotator AimRotation = OwningCharacter->GetBaseAimRotation();
	FRotator ControlRotation = OwningCharacter->GetControlRotation();
	FRotator TargetRotation = OwningCharacter->IsLocallyControlled() ? ControlRotation : AimRotation; 
	FRotator RootRotator = RootTransform.Rotator();

	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, RootRotator);
	AimPitch = Delta.Pitch;
	
	if (OwningCharacter->GetLocalRole() == ROLE_SimulatedProxy)
	{
		// Ignore large error caused by actor rotation de-sync when not strafing.
		// When UCharacterMovementComponent->bOrientRotationToMovement become true, actor rotate towards input direction and AimYaw become large.
		// In that case, we will use ReplicatedIndependentYaw, but it replicates slowly.
		float YawDiff = FRotator::NormalizeAxis(Delta.Yaw - AimYaw_Last);
		if (FMath::Abs(YawDiff) < 70.f)
		{
			AimYaw = Delta.Yaw;
		}
		
		FZodiacReplicatedIndependentYaw IndependentYaw = OwningCharacter->GetReplicatedIndependentYaw();
		if (IndependentYaw.bIsAllowed)
		{
			FRotator IndependentAimRotation(0.f, IndependentYaw.GetUnpackedYaw(), 0.f);
			AimYaw = FRotator::NormalizeAxis(IndependentYaw.GetUnpackedYaw() - RootRotator.Yaw);
		}
	}
	else
	{
		AimYaw = Delta.Yaw;
	}
}

void UZodiacHostAnimInstance::UpdateHeroData()
{
	if (AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(OwningCharacter))
	{
		if (AZodiacHeroCharacter* Hero = HostCharacter->GetHero())
		{
			if (UZodiacHeroAnimInstance* HeroAnimInstance = Hero->GetHeroAnimInstance())
			{
				bIsFocus = HeroAnimInstance->GetIsFocus();
				bIsADS = HeroAnimInstance->GetIsADS();
			}
		}	
	}
}

void UZodiacHostAnimInstance::UpdateVelocityData()
{
	Velocity_Last = Velocity;
	Velocity = ZodiacCharMovComp->Velocity;
	Speed2D = UKismetMathLibrary::VSizeXY(Velocity);
	bHasVelocity = Speed2D > 5.0f;
	if (bHasVelocity) Velocity_LastNonZero = Velocity;

	FVector Velocity2D = Velocity * FVector(1.0f, 1.0f, 0.f);
	LocalVelocity2D = WorldRotation.UnrotateVector(Velocity2D);
}

void UZodiacHostAnimInstance::UpdateAccelerationData(float DeltaSeconds)
{
	AccelerationFromVelocityDiff = (Velocity - Velocity_Last) / (FMath::Max(DeltaSeconds, 0.001f));
	
	Acceleration = ZodiacCharMovComp->GetCurrentAcceleration();
	AccelerationAmount = Acceleration.Length() / ZodiacCharMovComp->MaxAcceleration;
	bHasAcceleration = AccelerationAmount > 0;

	FVector Acceleration2D = Acceleration * FVector(1.0f, 1.0f, 0.f);
	LocalAcceleration2D = WorldRotation.UnrotateVector(Acceleration2D);
}

void UZodiacHostAnimInstance::OnStatusChanged(FGameplayTag Tag, bool bHasTag)
{
	if (Tag == ZodiacGameplayTags::Status_Death)
	{
		bIsDead = bHasTag;
	}
	else if (Tag == ZodiacGameplayTags::Status_Stun)
	{
		bIsStun = bHasTag;
	}
}

FVector UZodiacHostAnimInstance::GetTranslationOffset() const
{
	return (OwningCharacter) ? (RootTransform.GetLocation() - OwningCharacter->GetMesh()->GetComponentLocation()) : FVector(); 
}

void UZodiacHostAnimInstance::UpdateGait()
{
	if (ZodiacCharMovComp)
	{
		Gait_LastFrame = Gait;
		
		EMovementMode MovementMode = ZodiacCharMovComp->MovementMode;

		if (MovementMode == MOVE_Walking)
		{
			switch (ZodiacCharMovComp->GetExtendedMovementMode())
			{
			case EZodiacExtendedMovementMode::Running:
				Gait = Gait_Run;
				return;

			case EZodiacExtendedMovementMode::Sprinting:
				Gait = Gait_Sprint;
				return;
				
			case EZodiacExtendedMovementMode::Walking:
				Gait = Gait_Walk;
			default:
				return;
			}
		}
	}
}

void UZodiacHostAnimInstance::UpdateMotionMatchingPoseSelection(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result;
	FMotionMatchingAnimNodeReference MotionMatchingNode = UMotionMatchingAnimNodeLibrary::ConvertToMotionMatchingNode(Node, Result);
	if (Result == EAnimNodeReferenceConversionResult::Succeeded)
	{
		FPoseSearchBlueprintResult SearchResult;
		bool bIsResultValid;
		UMotionMatchingAnimNodeLibrary::GetMotionMatchingSearchResult(MotionMatchingNode, SearchResult, bIsResultValid);
		if (bIsResultValid)
		{
			CurrentSelectedDatabase = SearchResult.SelectedDatabase;

#if WITH_EDITORONLY_DATA
		if (ZodiacConsoleVariables::CVarLogMotionMatching.GetValueOnAnyThread())
		{
			Debug_SelectedAnimSequence = Cast<UAnimSequence>(SearchResult.SelectedAnimation);
		}
#endif
		}
	}
}
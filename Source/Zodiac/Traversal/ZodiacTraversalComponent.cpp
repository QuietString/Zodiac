// the.quiet.string@gmail.com
// Based on Epic Games, Animation Sample project.

#include "ZodiacTraversalComponent.h"

#include "ZodiacInteractionTransformInterface.h"
#include "ZodiacLogChannels.h"
#include "ZodiacTraversableActor.h"
#include "ZodiacTraversalTypes.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacTraversalComponent)

#define LOCTEXT_NAMESPACE "ZodiacTraversal"

UZodiacTraversalComponent::UZodiacTraversalComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UZodiacTraversalComponent::OnRegister()
{
	Super::OnRegister();

	TObjectPtr<ACharacter> OwningCharacter = GetPawn<ACharacter>();
	ensureMsgf(OwningCharacter, TEXT("Traversal component should be attached to an ACharacter"));
}

void UZodiacTraversalComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TraversalCheckResult);
}

bool UZodiacTraversalComponent::TryTraversalActionFromAbility(bool bIsInAir)
{
	bool bTraversalActionCheckFailed;
	bool bMontageSelectionFailed;
	FText FailReason;
	TryTraversalAction(bIsInAir, bTraversalActionCheckFailed, bMontageSelectionFailed, FailReason);

#if WITH_EDITOR
	if (ZodiacConsoleVariables::CVarTraversalEnableLogging.GetValueOnAnyThread())
	{
		if (bMontageSelectionFailed || bTraversalActionCheckFailed)
		{
			UE_LOG(LogZodiacTraversal, Warning, TEXT("Traversal action failed reason: %s"), *FailReason.ToString());	
		}
	}
#endif
	
	return !bTraversalActionCheckFailed && !bMontageSelectionFailed;
}

FZodiacTraversalCheckResult UZodiacTraversalComponent::TryTraversalAction(bool bIsInAr, bool& bTraversalCheckFailed, bool& bMontageSelectionFailed,
                                                                          FText& FailReason)
{
	FZodiacTraversalCheckResult CheckResult;
	
	bool bDrawDebug = false;
	bool bDrawFindBlockTrace = false;
	bool bDrawCeilingTrace = false;
	bool bDrawBackLedgeTrace = false;
	bool bDrawFloorTrace = false;
	
#if WITH_EDITOR
	bDrawDebug = ZodiacConsoleVariables::CVarTraversalEnableLogging.GetValueOnAnyThread();
	bDrawFindBlockTrace = bDrawDebug && ZodiacConsoleVariables::CVarTraversalDrawFindBlockTrace.GetValueOnAnyThread();
	bDrawCeilingTrace = bDrawDebug && ZodiacConsoleVariables::CVarTraversalDrawCeilingTrace.GetValueOnAnyThread();
	bDrawBackLedgeTrace = bDrawDebug && ZodiacConsoleVariables::CVarTraversalDrawBackLedgeTrace.GetValueOnAnyThread();
	bDrawFloorTrace = bDrawDebug && ZodiacConsoleVariables::CVarTraversalDrawFloorTrace.GetValueOnAnyThread();
#endif

	// Step 1: Cache some important values for use later in the function.
	TObjectPtr<ACharacter> OwningCharacter = GetPawn<ACharacter>();
	float CapsuleRadius = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float CapsuleHalfHeight = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector ActorLocation = OwningCharacter->GetActorLocation();
	FVector ActorForwardVector = OwningCharacter->GetActorForwardVector();
	
	// Step 2.1: Find a Traversable Level Block. If found, set the Hit Component, if not, exit the function.
	float TraceForwardDistance = GetTraversalForwardTraceDistance(bIsInAr);
	FVector TraceStart = ActorLocation;
	FVector TraceEnd = ActorLocation + ActorForwardVector * TraceForwardDistance;
	FHitResult TraversalObjectHit;
	
	if (!CapsuleTrace(TraceStart, TraceEnd, bDrawFindBlockTrace, OUT TraversalObjectHit, CapsuleRadius, CapsuleHalfHeight))
	{
		FailReason = LOCTEXT("TraversalFailed", "No trace hit");
		bTraversalCheckFailed = true;
		return CheckResult;
	}
	
	AZodiacTraversableActor* TraversableObject = Cast<AZodiacTraversableActor>(TraversalObjectHit.GetActor());
	if (TraversableObject == nullptr)
	{
		FailReason = LOCTEXT("TraversalFailed", "No traversal object found");
		bTraversalCheckFailed = true;
		return CheckResult;
	}

	// Step 2.2: If a traversable level block was found, get the front and back ledge transforms from it.
	TraversableObject->GetLedgeTransforms(TraversalObjectHit.ImpactPoint, ActorLocation, OUT CheckResult);
	CheckResult.HitComponent = TraversalObjectHit.Component;

	// Step 3.1 If the traversable level block has a valid front ledge, continue the function. If not, exit early.
	if (!CheckResult.bHasFrontLedge)
	{
		FailReason = LOCTEXT("TraversalFailed", "No front ledge found");
		bTraversalCheckFailed = true;
		return CheckResult;
	}

	// Step 3.2: Check if there is enough room above for traversal action.
	FVector CeilingCheckStartLocation = ActorLocation;
	FVector CeilingCheckEndLocation = CheckResult.FrontLedgeLocation
										+ FVector(0.0f, 0.0f, CapsuleHalfHeight + 2)
										+ CheckResult.FrontLedgeNormal * (CapsuleRadius + 2); // end trace before capsule collide with FrontLedge
	FHitResult CeilingHit;
	CapsuleTrace(CeilingCheckStartLocation, CeilingCheckEndLocation, bDrawCeilingTrace, CeilingHit, CapsuleRadius, CapsuleHalfHeight);
	if (CeilingHit.bBlockingHit || CeilingHit.bStartPenetrating)
	{
		FailReason = LOCTEXT("TraversalFailed", "Too close ceiling for traversal action");
		bTraversalCheckFailed = true;
		return 	CheckResult;
	}
	
	// Step 3.3: Save the height of the obstacle using the delta between the actor and front ledge transform.
	CheckResult.ObstacleHeight = FMath::Abs(((ActorLocation - FVector(0.0f, 0.0f, CapsuleHalfHeight)) - CheckResult.FrontLedgeLocation).Z);

	// Step 3.4: Perform a trace across the top of the obstacle from the front ledge to the back ledge to see if theres room for the actor to move across it.
	FVector FrontRoomCheckStartLocation = CeilingCheckEndLocation;
	FVector FrontRoomCheckEndLocation = CheckResult.BackLedgeLocation
										+ CheckResult.BackLedgeNormal * (CapsuleRadius + 2)
										+ FVector(0.0f, 0.0f, CapsuleHalfHeight + 2);
	FHitResult BackLedgeHit;
	if (CapsuleTrace(FrontRoomCheckStartLocation, FrontRoomCheckEndLocation, bDrawBackLedgeTrace, BackLedgeHit, CapsuleRadius, CapsuleHalfHeight))
	{
		// Step 3.5: If there is not room, save the obstacle depth using the difference between the front ledge and the trace impact point, and invalidate the back ledge.
		CheckResult.ObstacleDepth = (BackLedgeHit.ImpactPoint - CheckResult.FrontLedgeLocation).Length();
		CheckResult.bHasBackLedge = false;
	}
	else
	{
		// Step 3.5: If there is room, save the obstacle depth using the difference between the front and back ledge locations.
		CheckResult.ObstacleDepth = (CheckResult.FrontLedgeLocation - CheckResult.BackLedgeLocation).Length();
	}

	// Step 3.6: Trace downward from the back ledge location (using the height of the obstacle for the distance) to find the floor.
	// If there is a floor, save its location and the back ledges height (using the distance between the back ledge and the floor).
	// If no floor was found, invalidate the back floor.
	FVector FloorCheckStartLocation = FrontRoomCheckEndLocation;
	FVector FloorCheckEndLocation = CheckResult.BackLedgeLocation
									+ CheckResult.BackLedgeNormal * (CapsuleRadius + 2)
									- FVector(0.0f, 0.0f, CheckResult.ObstacleHeight - CapsuleHalfHeight + 50.0f);
	FHitResult FloorHit;
	if (CapsuleTrace(FloorCheckStartLocation, FloorCheckEndLocation, bDrawFloorTrace, OUT FloorHit, CapsuleRadius, CapsuleHalfHeight))
	{
		CheckResult.bHasBackFloor = true;
		CheckResult.BackFloorLocation = FloorHit.ImpactPoint;
		CheckResult.BackLedgeHeight = FMath::Abs((FloorHit.ImpactPoint - CheckResult.BackLedgeLocation).Z);
	}
	else
	{
		CheckResult.bHasBackFloor = false;
	}

	if (!DetermineTraversalType(CheckResult))
	{
		FailReason = LOCTEXT("TraversalCheckFailed", "Couldn't determine proper action type.");
		bTraversalCheckFailed = true;
		return 	CheckResult;
	}

	CheckResult.Speed = OwningCharacter->GetCharacterMovement()->Velocity.Length();
	
	if (!FindMatchingAnimMontage(CheckResult))
	{
		FailReason = LOCTEXT("TraversalFailed", "Couldn't find matching anim montage");
		bMontageSelectionFailed = true;
		return 	CheckResult;
	}

	if (OwningCharacter->HasAuthority())
	{
		TraversalCheckResult = CheckResult;
		PerformTraversalAction(TraversalCheckResult);
	}

	return 	CheckResult;
}

float UZodiacTraversalComponent::GetTraversalForwardTraceDistance(bool bIsInAir) const
{
	TObjectPtr<ACharacter> OwningCharacter = GetPawn<ACharacter>();
	FRotator ActorRotation = OwningCharacter->GetActorRotation();

	if (bIsInAir)
	{
		return AirTraversalRange.Y;
	}
	else
	{
		float ForwardDistance = ActorRotation.UnrotateVector(OwningCharacter->GetCharacterMovement()->Velocity).X;
		float ForwardDistanceClamped = FMath::GetMappedRangeValueClamped(FVector2f(0.0f, 500.0f), FVector2f(75.0f, 350.0f), ForwardDistance);
		return ForwardDistanceClamped;
	}
}

bool UZodiacTraversalComponent::CapsuleTrace(const FVector& TraceStart, const FVector& TraceEnd, bool bDrawDebug, FHitResult& OutHit, float CapsuleRadius, float CapsuleHalfHeight)
{
	UWorld* World = GetWorld();
	check(World);
	
	ETraceTypeQuery TraceTypeQuery = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	TArray<AActor*> ActorsToIgnore;
	EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::Type::ForDuration : EDrawDebugTrace::Type::None;
	float DrawTime = 10.0f;
	return UKismetSystemLibrary::CapsuleTraceSingle(World, TraceStart, TraceEnd, CapsuleRadius, CapsuleHalfHeight, TraceTypeQuery, false, ActorsToIgnore, DrawDebugType, OutHit, true, FLinearColor::Red, FLinearColor::Green, DrawTime);
}

bool UZodiacTraversalComponent::DetermineTraversalType(FZodiacTraversalCheckResult& CheckResult)
{
	FZodiacTraversalCheckResult& R = CheckResult;

	bool bLongEnoughToStepOn = R.ObstacleDepth >= 59.0f;
	
	bool CanVault = UKismetMathLibrary::InRange_FloatFloat(R.ObstacleHeight, VaultHeightRange.X, VaultHeightRange.Y);
	if (R.bHasFrontLedge && R.bHasBackLedge && !R.bHasBackFloor && CanVault && !bLongEnoughToStepOn)
	{
		R.ActionType = EZodiacTraversalActionType::Vault;
		return true;
	}

	bool CanHurdle = UKismetMathLibrary::InRange_FloatFloat(R.ObstacleHeight, HurdleHeightRange.X, HurdleHeightRange.Y);
	if (R.bHasFrontLedge && R.bHasBackLedge && R.bHasBackFloor && CanHurdle && !bLongEnoughToStepOn && R.BackLedgeHeight > 50.0f)
	{
		R.ActionType = EZodiacTraversalActionType::Hurdle;
		return true;
	}

	bool CanMantle = UKismetMathLibrary::InRange_FloatFloat(R.ObstacleHeight, MantleHeightRange.X, MantleHeightRange.Y);
	if (R.bHasFrontLedge && CanMantle && bLongEnoughToStepOn)
	{
		R.ActionType = EZodiacTraversalActionType::Mantle;
		return true;
	}

	R.ActionType = EZodiacTraversalActionType::None;
	return false;
}

bool UZodiacTraversalComponent::FindMatchingAnimMontage(FZodiacTraversalCheckResult& CheckResult)
{
	TObjectPtr<ACharacter> OwningCharacter = GetPawnChecked<ACharacter>();
	if (UAnimInstance* AnimInstance = OwningCharacter->GetMesh()->GetAnimInstance())
	{
		if (AnimInstance->Implements<UZodiacInteractionTransformInterface>())
		{
			FVector Translation = CheckResult.FrontLedgeLocation;
			FRotator Rotation = UKismetMathLibrary::MakeRotFromZ(CheckResult.FrontLedgeNormal);
			FVector Scale = FVector(1.0f, 1.0f, 1.0f);

			FTransform InteractionTransform(Rotation, Translation, Scale);
			IZodiacInteractionTransformInterface::Execute_SetInteractionTransform(AnimInstance, InteractionTransform);

			FZodiacTraversalChooserParams ChooserParams;
			ChooserParams.ActionType = CheckResult.ActionType;
			ChooserParams.Speed = CheckResult.Speed;
			ChooserParams.ObstacleHeight = CheckResult.ObstacleHeight;
			ChooserParams.ObstacleDepth = CheckResult.ObstacleDepth;
			
			float StartTime;
			float PlayRate;
			CheckResult.ChosenMontage = RunChooser(ChooserParams, StartTime, PlayRate);
			CheckResult.StartTime = StartTime;
			CheckResult.PlayRate = PlayRate;

			if (CheckResult.ChosenMontage.IsValid())
			{
				return true;
			}
		}
	}

	return false;
}

void UZodiacTraversalComponent::OnRepTraversalCheckResult()
{
	PerformTraversalAction(TraversalCheckResult);
}

#undef LOCTEXT_NAMESPACE

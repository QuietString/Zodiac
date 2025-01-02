// the.quiet.string@gmail.com
// Based on Epic Games, Animation Sample project.

#include "ZodiacTraversalComponent.h"

#include "AbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacInteractionTransformInterface.h"
#include "ZodiacLogChannels.h"
#include "ZodiacTraversableActorComponent.h"
#include "ZodiacTraversalTypes.h"
#include "Character/ZodiacCharacter.h"
#include "Character/ZodiacCharacterMovementComponent.h"
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
	PrimaryComponentTick.bCanEverTick = true;
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

void UZodiacTraversalComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UZodiacTraversalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ACharacter* Character = GetPawn<ACharacter>())
	{
		if (UCharacterMovementComponent* CharMovComp = Character->GetCharacterMovement())
		{
			if (bEnableFindLedgeOnTick && Character->IsLocallyViewed())
			{
				if (CharMovComp->CustomMovementMode != Move_Custom_Traversal)
				{
					bool bIsInAir = !Character->GetCharacterMovement()->IsMovingOnGround();
					FText FailReason;
					FZodiacTraversalCheckResult Result;
					FVector LastLocation;
					bool bLedgeFound = CheckFrontLedge(bIsInAir, Result, FailReason, LastLocation, true);
			
					OnFrontLedgeChecked(bLedgeFound, Result.FrontLedgeLocation, Result.FrontLedgeNormal);
			
#if WITH_EDITOR
					if (ZodiacConsoleVariables::CVarTraversalDrawDebug.GetValueOnAnyThread())
					{
						if (GEngine)
						{
							GEngine->AddOnScreenDebugMessage(9135, 0, FColor::Green, FailReason.ToString());
						}
					}
#endif	
				}
				else
				{
					OnFrontLedgeChecked(false, FVector(), FVector());
				}
			}
		}
	}
}

bool UZodiacTraversalComponent::CanTraversalAction(FText& FailReason)
{
	ACharacter* Character = GetPawn<ACharacter>();
	if (!Character)
	{
		return false;
	}

	UCharacterMovementComponent* CharMovComp = Character->GetCharacterMovement();
	if (!CharMovComp)
	{
		return false;
	}

	if (CharMovComp->MovementMode == MOVE_Flying)
	{
		return false;
	}

	if (bHasCached)
	{
		return true;
	}
	
	bool bDrawDebug = false;
	bool bDrawBackLedgeTrace = false;
	bool bDrawFloorTrace = false;
	float DebugDuration = 0.0f;

#if WITH_EDITOR
	bDrawDebug = ZodiacConsoleVariables::CVarTraversalDrawDebug.GetValueOnAnyThread();
	int32 DebugLevel = ZodiacConsoleVariables::CVarTraversalDebugLevel.GetValueOnAnyThread();

	bDrawBackLedgeTrace = bDrawDebug && DebugLevel > 2;
	bDrawFloorTrace = bDrawDebug && DebugLevel > 3;
	DebugDuration = ZodiacConsoleVariables::CVarTraversalDrawDuration.GetValueOnAnyThread();
#endif
	
	FZodiacTraversalCheckResult Result;
	
	bool bIsInAir = !Character->GetCharacterMovement()->IsMovingOnGround();
	Result.bIsMidAir = bIsInAir;
	
	FVector CeilingCheckEndLocation;
	if (!CheckFrontLedge(bIsInAir,Result,FailReason, CeilingCheckEndLocation, false))
	{
		return false;
	}
	
	TObjectPtr<ACharacter> OwningCharacter = GetPawn<ACharacter>();
	float CapsuleRadius = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float CapsuleHalfHeight = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector ActorLocation = OwningCharacter->GetActorLocation();
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwningCharacter);
	ActorsToIgnore.Add(Result.HitComponent->GetOwner());
	OwningCharacter->GetAttachedActors(ActorsToIgnore, false);

	// Step 3.3: Save the height of the obstacle using the delta between the actor and front ledge transform.
	Result.ObstacleHeight = FMath::Abs(((ActorLocation - FVector(0.0f, 0.0f, CapsuleHalfHeight)) - Result.FrontLedgeLocation).Z);

	// Step 3.4: Perform a trace across the top of the obstacle from the front ledge to the back ledge to see if theres room for the actor to move across it.
	FVector FrontRoomCheckStartLocation = CeilingCheckEndLocation;
	FVector FrontRoomCheckEndLocation = Result.BackLedgeLocation
										+ Result.BackLedgeNormal * (CapsuleRadius + 2)
										+ FVector(0.0f, 0.0f, CapsuleHalfHeight + 2);
	FHitResult BackLedgeHit;
	if (Result.bHasBackLedge && CapsuleTrace(FrontRoomCheckStartLocation, FrontRoomCheckEndLocation, BackLedgeHit, CapsuleRadius, CapsuleHalfHeight, bDrawBackLedgeTrace, DebugDuration, false, ActorsToIgnore))
	{
		// Step 3.5: If there is not room, save the obstacle depth using the difference between the front ledge and the trace impact point, and invalidate the back ledge.
		Result.ObstacleDepth = (BackLedgeHit.ImpactPoint - Result.FrontLedgeLocation).Length();
		Result.bHasBackLedge = false;
	}
	else
	{
		// Step 3.5: If there is room, save the obstacle depth using the difference between the front and back ledge locations.
		Result.ObstacleDepth = (Result.FrontLedgeLocation - Result.BackLedgeLocation).Length();
	}

	// Step 3.6: Trace downward from the back ledge location (using the height of the obstacle for the distance) to find the floor.
	// If there is a floor, save its location and the back ledges height (using the distance between the back ledge and the floor).
	// If no floor was found, invalidate the back floor.
	FVector FloorCheckStartLocation = FrontRoomCheckEndLocation;
	FVector FloorCheckEndLocation = Result.BackLedgeLocation
									+ Result.BackLedgeNormal * (CapsuleRadius + 2)
									- FVector(0.0f, 0.0f, Result.ObstacleHeight - CapsuleHalfHeight + 50.0f);
	FHitResult FloorHit;
	if (CapsuleTrace(FloorCheckStartLocation, FloorCheckEndLocation, FloorHit, OUT CapsuleRadius, CapsuleHalfHeight, bDrawFloorTrace, DebugDuration, false, ActorsToIgnore))
	{
		Result.bHasBackFloor = true;
		Result.BackFloorLocation = FloorHit.ImpactPoint;
		Result.BackLedgeHeight = FMath::Abs((FloorHit.ImpactPoint - Result.BackLedgeLocation).Z);
	}
	else
	{
		Result.bHasBackFloor = false;
	}

	if (!DetermineTraversalType(Result))
	{
		FailReason = LOCTEXT("TraversalCheckFailed", "Couldn't determine proper action type.");
		return 	false;
	}

	Result.Speed = OwningCharacter->GetCharacterMovement()->Velocity.Length();
	
	if (!FindMatchingAnimMontage(Result))
	{
		FailReason = LOCTEXT("TraversalFailed", "Couldn't find matching anim montage");
		return 	false;
	}
	
	CheckResultCached = Result;
	bHasCached = true;
	
	return true;
}

void UZodiacTraversalComponent::TryActivateTraversalAbility()
{
	// PerformTraversalActionFromAbility() will be called from ASC.
	if (AZodiacCharacter* ZodiacCharacter = GetPawn<AZodiacCharacter>())
	{
		if (UAbilitySystemComponent* ASC = ZodiacCharacter->GetAbilitySystemComponent())
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();

			FGameplayEventData Payload;
			Payload.EventTag = ZodiacGameplayTags::Event_Ability_Traversal;
			Payload.Instigator = nullptr;
			Payload.Target = ASC->GetAvatarActor();
			Payload.ContextHandle = ContextHandle;
			Payload.EventMagnitude = 1;

			FScopedPredictionWindow NewScopedWindow(ASC, true);
			ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
		}
	}
}

void UZodiacTraversalComponent::PerformTraversalActionFromAbility()
{
	TraversalCheckResult = CheckResultCached;
	PerformTraversalAction(TraversalCheckResult);

	// Clear cache
	CheckResultCached = FZodiacTraversalCheckResult();
	bHasCached = false;
}

bool UZodiacTraversalComponent::CheckFrontLedge(bool bIsInAir, FZodiacTraversalCheckResult& Result, FText& FailReason, FVector& LastTraceLocation, bool bIsTicked)
{
	bool bDrawDebug = false;
	bool bDrawFindBlockTrace = false;
	bool bDrawCeilingTrace = false;
	float DebugDuration = 0.0f;

#if WITH_EDITOR
	bDrawDebug = ZodiacConsoleVariables::CVarTraversalDrawDebug.GetValueOnAnyThread();
	int32 DebugLevel = ZodiacConsoleVariables::CVarTraversalDebugLevel.GetValueOnAnyThread();

	bDrawFindBlockTrace = bDrawDebug && (DebugLevel > 0);
	bDrawCeilingTrace = bDrawDebug && (DebugLevel > 1);
	DebugDuration = ZodiacConsoleVariables::CVarTraversalDrawDuration.GetValueOnAnyThread();
#endif

	// Step 1: Cache some important values for use later in the function.
	TObjectPtr<ACharacter> OwningCharacter = GetPawn<ACharacter>();
	float CapsuleRadius = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float CapsuleHalfHeight = OwningCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector ActorLocation = OwningCharacter->GetActorLocation();
	FVector ActorForwardVector = OwningCharacter->GetActorForwardVector();
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwningCharacter);
	OwningCharacter->GetAttachedActors(ActorsToIgnore, false);
	
	// Step 2.1: Find a Traversable Level Block. If found, set the Hit Component, if not, exit the function.
	float TraceForwardDistance = GetTraversalForwardTraceDistance(bIsInAir);
	FVector TraceStart = ActorLocation;
	FVector TraceEnd = ActorLocation + ActorForwardVector * TraceForwardDistance;
	FHitResult TraversalObjectHit;

	Result.bIsMidAir = bIsInAir;
	
	if (!CapsuleTrace(TraceStart, TraceEnd, TraversalObjectHit, OUT CapsuleRadius, CapsuleHalfHeight, bDrawFindBlockTrace, DebugDuration, bIsTicked, ActorsToIgnore))
	{
		FailReason = LOCTEXT("TraversalFailed", "No trace hit");
		return false;
	}

	AActor* HitActor = TraversalObjectHit.GetActor();
	if (UZodiacTraversableActorComponent* TraversableActorComponent = Cast<UZodiacTraversableActorComponent>(TraversalObjectHit.GetActor()->GetComponentByClass(UZodiacTraversableActorComponent::StaticClass())))
	{
		// Step 2.2: If a traversable level block was found, get the front and back ledge transforms from it.
		TraversableActorComponent->GetLedgeTransforms(TraversalObjectHit.ImpactPoint, ActorLocation, OUT Result);
		Result.HitComponent = TraversalObjectHit.Component;
		ActorsToIgnore.Add(HitActor);
	}
	else
	{
		FailReason = LOCTEXT("TraversalFailed", "No traversal object found");
		return false;
	}
	
	// Step 3.1 If the traversable level block has a valid front ledge, continue the function. If not, exit early.
	if (!Result.bHasFrontLedge)
	{
		FailReason = LOCTEXT("TraversalFailed", "No front ledge found");
		return false;
	}
	
	if (Result.FrontLedgeNormal.Dot(ActorForwardVector) > -0.7f)
	{
		FailReason = LOCTEXT("TraversalFailed", "Character is not facing toward a front ledge");
		return false;
	}
	
	// Step 3.2: Check if there is enough room above for traversal action.
	FVector CeilingCheckStartLocation = ActorLocation;
	FVector CeilingCheckEndLocation = Result.FrontLedgeLocation
										+ FVector(0.0f, 0.0f, CapsuleHalfHeight + 2)
										+ Result.FrontLedgeNormal * (CapsuleRadius + 2); // end trace before capsule collide with FrontLedge
	FHitResult CeilingHit;
	CapsuleTrace(CeilingCheckStartLocation, CeilingCheckEndLocation, CeilingHit, CapsuleRadius, CapsuleHalfHeight, bDrawCeilingTrace, DebugDuration, bIsTicked, ActorsToIgnore);
	if (CeilingHit.bBlockingHit || CeilingHit.bStartPenetrating)
	{
		FailReason = FText::Format(LOCTEXT("TraversalFailed", "Too close ceiling for traversal action. Hit component: {0}"), FText::FromString(CeilingHit.Component->GetName()));
		return 	false;
	}

	LastTraceLocation = CeilingCheckEndLocation; 
	return true;
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
		float ForwardDistanceClamped = FMath::GetMappedRangeValueClamped(FVector2f(0.0f, 500.0f), GroundForwardTraceRange, ForwardDistance);
		return ForwardDistanceClamped;
	}
}

bool UZodiacTraversalComponent::CapsuleTrace(const FVector& TraceStart, const FVector& TraceEnd, FHitResult& OutHit, float CapsuleRadius, float CapsuleHalfHeight, bool bDrawDebug, float
                                             DebugDuration, bool bIsTicked, const TArray<AActor*>& ActorsToIgnore)
{
	UWorld* World = GetWorld();
	check(World);
	
	ETraceTypeQuery TraceTypeQuery = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::Type::ForDuration : EDrawDebugTrace::Type::None;
	float DrawTime = bIsTicked ? 0.0f : DebugDuration;
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

	bool CanGroundMantle = UKismetMathLibrary::InRange_FloatFloat(R.ObstacleHeight, MantleHeightRange.X, MantleHeightRange.Y);
	if (R.bHasFrontLedge && CanGroundMantle && bLongEnoughToStepOn && !CheckResult.bIsMidAir)
	{
		R.ActionType = EZodiacTraversalActionType::Mantle;
		return true;
	}

	bool CanMidAirMantle = UKismetMathLibrary::InRange_FloatFloat(R.ObstacleHeight, MidAirMantleHeightRange.X, MidAirMantleHeightRange.Y);
	if (R.bHasFrontLedge && CanMidAirMantle && bLongEnoughToStepOn && CheckResult.bIsMidAir)
	{
		R.ActionType = EZodiacTraversalActionType::MidAirMantle;
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
			ChooserParams.bIsMidAir = CheckResult.bIsMidAir;
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

void UZodiacTraversalComponent::K2_NotifyTraversalActionFinished()
{
	if (OnTraversalFinished.IsBound())
	{
		OnTraversalFinished.Execute();
		OnTraversalFinished.Unbind();	
	}
}

void UZodiacTraversalComponent::OnRepTraversalCheckResult()
{
	PerformTraversalAction(TraversalCheckResult);

	// Clear cache
	CheckResultCached = FZodiacTraversalCheckResult();
	bHasCached = false;
}

#undef LOCTEXT_NAMESPACE

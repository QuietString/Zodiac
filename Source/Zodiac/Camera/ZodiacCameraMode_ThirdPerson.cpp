// Copyright Epic Games, Inc. All Rights Reserved.
// the.quiet.string@gmail.com

#include "ZodiacCameraMode_ThirdPerson.h"
#include "Camera/ZodiacCameraMode.h"
#include "Components/PrimitiveComponent.h"
#include "Camera/ZodiacPenetrationAvoidanceFeeler.h"
#include "Curves/CurveVector.h"
#include "Engine/Canvas.h"
#include "GameFramework/CameraBlockingVolume.h"
#include "ZodiacCameraAssistInterface.h"
#include "ZodiacCameraComponent.h"
#include "Character/ZodiacMonster.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Controller.h"
#include "Math/RotationMatrix.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacCameraMode_ThirdPerson)

namespace ZodiacCameraMode_ThirdPerson_Statics
{
	static const FName NAME_IgnoreCameraCollision = TEXT("IgnoreCameraCollision");
}

UZodiacCameraMode_ThirdPerson::UZodiacCameraMode_ThirdPerson()
{
	TargetOffsetCurve = nullptr;

	PenetrationAvoidanceFeelers.Add(FZodiacPenetrationAvoidanceFeeler(FRotator(+00.0f, +00.0f, 0.0f), 1.00f, 1.00f, 14.f, 0));
	PenetrationAvoidanceFeelers.Add(FZodiacPenetrationAvoidanceFeeler(FRotator(+00.0f, +16.0f, 0.0f), 0.75f, 0.75f, 00.f, 3));
	PenetrationAvoidanceFeelers.Add(FZodiacPenetrationAvoidanceFeeler(FRotator(+00.0f, -16.0f, 0.0f), 0.75f, 0.75f, 00.f, 3));
	PenetrationAvoidanceFeelers.Add(FZodiacPenetrationAvoidanceFeeler(FRotator(+00.0f, +32.0f, 0.0f), 0.50f, 0.50f, 00.f, 5));
	PenetrationAvoidanceFeelers.Add(FZodiacPenetrationAvoidanceFeeler(FRotator(+00.0f, -32.0f, 0.0f), 0.50f, 0.50f, 00.f, 5));
	PenetrationAvoidanceFeelers.Add(FZodiacPenetrationAvoidanceFeeler(FRotator(+20.0f, +00.0f, 0.0f), 1.00f, 1.00f, 00.f, 4));
	PenetrationAvoidanceFeelers.Add(FZodiacPenetrationAvoidanceFeeler(FRotator(-20.0f, +00.0f, 0.0f), 0.50f, 0.50f, 00.f, 4));

	CloseContactBlendExponent = 2.f;
	CloseContactBlendFunction = EZodiacCameraModeBlendFunction::EaseInOut;
	CloseContactBlendInTime = 0.5f;
	CloseContactBlendOutTime = 1.f;
	
	CloseContactFeelers.Add(FZodiacCloseContactFeeler(250, 15, 5));
}

void UZodiacCameraMode_ThirdPerson::UpdateView(float DeltaTime)
{
    FVector PivotLocation = GetPivotLocation();
    FRotator PivotRotation = GetPivotRotation();

    PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

    View.Location = PivotLocation;
    View.Rotation = PivotRotation;
    View.ControlRotation = View.Rotation;
    View.FieldOfView = FieldOfView;

    UpdateTargetOffsetCurve(DeltaTime, PivotRotation);

    // Update camera location with the calculated TargetOffset
    View.Location = PivotLocation + PivotRotation.RotateVector(TargetOffset);

    // Adjust final desired camera location to prevent any penetration
    UpdatePreventPenetration(DeltaTime);
}

void UZodiacCameraMode_ThirdPerson::UpdateTargetOffsetCurve(float DeltaTime, FRotator PivotRotation)
{
	bHasCloseContact = CheckCloseContact();

	// Check for change in close contact status
	if (bHasCloseContact != bPreviousCloseContact)
	{
		// Close contact status has changed
		bIsBlending = true;
		BlendElapsedTime = 0.0f;
		BlendDuration = bHasCloseContact ? CloseContactBlendInTime : CloseContactBlendOutTime;

		// Record the start and end offsets
		BlendStartOffset = TargetOffset;

		// Determine the BlendEndOffset based on the new close contact status
		if (bHasCloseContact)
		{
			if (CloseContactOffsetCurve)
			{
				BlendEndOffset = CloseContactOffsetCurve->GetVectorValue(PivotRotation.Pitch);
			}
		}
		else
		{
			if (TargetOffsetCurve)
			{
				BlendEndOffset = TargetOffsetCurve->GetVectorValue(PivotRotation.Pitch);
			}

			if (TargetOffsetCurveByYaw)
			{
				if (UZodiacCameraComponent* ZodiacCameraComponent = GetZodiacCameraComponent())
				{
					BlendEndOffset += TargetOffsetCurveByYaw->GetVectorValue(ZodiacCameraComponent->AimYaw);
				}
			}
		}
	}

	// Update previous close contact status
	bPreviousCloseContact = bHasCloseContact;

	if (bIsBlending)
	{
		// Update blend elapsed time
		BlendElapsedTime += DeltaTime;
		float OffsetCurveBlendAlpha = FMath::Clamp(BlendElapsedTime / BlendDuration, 0.0f, 1.0f);

		// Interpolate between start and end offsets
		SetCloseContactOffset(BlendStartOffset, BlendEndOffset, OffsetCurveBlendAlpha);
		
		// Check if blending is complete
		if (OffsetCurveBlendAlpha >= 1.0f)
		{
			bIsBlending = false;
		}
	}
	else
	{
		// No blending; use the appropriate offset directly
		if (bHasCloseContact)
		{
			if (CloseContactOffsetCurve)
			{
				TargetOffset = CloseContactOffsetCurve->GetVectorValue(PivotRotation.Pitch);
			}
		}
		else
		{
			if (TargetOffsetCurve)
			{
				TargetOffset = TargetOffsetCurve->GetVectorValue(PivotRotation.Pitch);
			}

			if (TargetOffsetCurveByYaw)
			{
				if (UZodiacCameraComponent* ZodiacCameraComponent = GetZodiacCameraComponent())
				{
					TargetOffset += TargetOffsetCurveByYaw->GetVectorValue(ZodiacCameraComponent->AimYaw);
				}
			}
		}
	}
}

void UZodiacCameraMode_ThirdPerson::DrawDebug(UCanvas* Canvas) const
{
	Super::DrawDebug(Canvas);

#if ENABLE_DRAW_DEBUG
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	for (int i = 0; i < DebugActorsHitDuringCameraPenetration.Num(); i++)
	{
		DisplayDebugManager.DrawString(
			FString::Printf(TEXT("HitActorDuringPenetration[%d]: %s")
				, i
				, *DebugActorsHitDuringCameraPenetration[i]->GetName()));
	}

	LastDrawDebugTime = GetWorld()->GetTimeSeconds();
#endif
}

void UZodiacCameraMode_ThirdPerson::UpdatePreventPenetration(float DeltaTime)
{
	if (!bPreventPenetration)
	{
		return;
	}

	AActor* TargetActor = GetTargetActor();

	APawn* TargetPawn = Cast<APawn>(TargetActor);
	AController* TargetController = TargetPawn ? TargetPawn->GetController() : nullptr;
	IZodiacCameraAssistInterface* TargetControllerAssist = Cast<IZodiacCameraAssistInterface>(TargetController);

	IZodiacCameraAssistInterface* TargetActorAssist = Cast<IZodiacCameraAssistInterface>(TargetActor);

	TOptional<AActor*> OptionalPPTarget = TargetActorAssist ? TargetActorAssist->GetCameraPreventPenetrationTarget() : TOptional<AActor*>();
	AActor* PPActor = OptionalPPTarget.IsSet() ? OptionalPPTarget.GetValue() : TargetActor;
	IZodiacCameraAssistInterface* PPActorAssist = OptionalPPTarget.IsSet() ? Cast<IZodiacCameraAssistInterface>(PPActor) : nullptr;

	const UPrimitiveComponent* PPActorRootComponent = Cast<UPrimitiveComponent>(PPActor->GetRootComponent());
	if (PPActorRootComponent)
	{
		// Attempt at picking SafeLocation automatically, so we reduce camera translation when aiming.
		// Our camera is our reticle, so we want to preserve our aim and keep that as steady and smooth as possible.
		// Pick closest point on capsule to our aim line.
		FVector ClosestPointOnLineToCapsuleCenter;
		FVector SafeLocation = PPActor->GetActorLocation();
		FMath::PointDistToLine(SafeLocation, View.Rotation.Vector(), View.Location, ClosestPointOnLineToCapsuleCenter);
		
		// Adjust Safe distance height to be same as aim line, but within capsule.
		float const PushInDistance = PenetrationAvoidanceFeelers[0].Extent + CollisionPushOutDistance;
		float const MaxHalfHeight = PPActor->GetSimpleCollisionHalfHeight() - PushInDistance;
		SafeLocation.Z = FMath::Clamp(ClosestPointOnLineToCapsuleCenter.Z, SafeLocation.Z - MaxHalfHeight, SafeLocation.Z + MaxHalfHeight);

		float DistanceSqr;
		PPActorRootComponent->GetSquaredDistanceToCollision(ClosestPointOnLineToCapsuleCenter, DistanceSqr, SafeLocation);
		// Push back inside capsule to avoid initial penetration when doing line checks.
		if (PenetrationAvoidanceFeelers.Num() > 0)
		{
			SafeLocation += (SafeLocation - ClosestPointOnLineToCapsuleCenter).GetSafeNormal() * PushInDistance;
		}
		
		// Then aim line to desired camera position
		bool const bSingleRayPenetrationCheck = !bDoPredictiveAvoidance;
		PreventCameraPenetration(*PPActor, SafeLocation, View.Location, DeltaTime, AimLineToDesiredPosBlockedPct, bSingleRayPenetrationCheck);

		IZodiacCameraAssistInterface* AssistArray[] = { TargetControllerAssist, TargetActorAssist, PPActorAssist };

		if (AimLineToDesiredPosBlockedPct < ReportPenetrationPercent)
		{
			for (IZodiacCameraAssistInterface* Assist : AssistArray)
			{
				if (Assist)
				{
					// camera is too close, tell the assists
					Assist->OnCameraPenetratingTarget();
				}
			}
		}
	}
}

void UZodiacCameraMode_ThirdPerson::PreventCameraPenetration(class AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly)
{
#if ENABLE_DRAW_DEBUG
	DebugActorsHitDuringCameraPenetration.Reset();
#endif

	float HardBlockedPct = DistBlockedPct;
	float SoftBlockedPct = DistBlockedPct;

	FVector BaseRay = CameraLoc - SafeLoc;
	FRotationMatrix BaseRayMatrix(BaseRay.Rotation());
	FVector BaseRayLocalUp, BaseRayLocalFwd, BaseRayLocalRight;

	BaseRayMatrix.GetScaledAxes(BaseRayLocalFwd, BaseRayLocalRight, BaseRayLocalUp);

	float DistBlockedPctThisFrame = 1.f;
	
	int32 const NumRaysToShoot = bSingleRayOnly ? FMath::Min(1, PenetrationAvoidanceFeelers.Num()) : PenetrationAvoidanceFeelers.Num();
	FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(CameraPen), false, nullptr/*PlayerCamera*/);

	SphereParams.AddIgnoredActor(&ViewTarget);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(0.f);
	UWorld* World = GetWorld();

	for (int32 RayIdx = 0; RayIdx < NumRaysToShoot; ++RayIdx)
	{
		FZodiacPenetrationAvoidanceFeeler& Feeler = PenetrationAvoidanceFeelers[RayIdx];
		if (Feeler.FramesUntilNextTrace <= 0)
		{
			// calc ray target
			FVector RayTarget;
			{
				FVector RotatedRay = BaseRay.RotateAngleAxis(Feeler.AdjustmentRot.Yaw, BaseRayLocalUp);
				RotatedRay = RotatedRay.RotateAngleAxis(Feeler.AdjustmentRot.Pitch, BaseRayLocalRight);
				RayTarget = SafeLoc + RotatedRay;
			}
			
			// cast for world and pawn hits separately.  this is so we can safely ignore the 
			// camera's target pawn
			SphereShape.Sphere.Radius = Feeler.Extent;
			ECollisionChannel TraceChannel = ECC_Camera;		//(Feeler.PawnWeight > 0.f) ? ECC_Pawn : ECC_Camera;

			// do multi-line check to make sure the hits we throw out aren't
			// masking real hits behind (these are important rays).

			// MT-> passing camera as actor so that camerablockingvolumes know when it's the camera doing traces
			FHitResult Hit;
			const bool bHit = World->SweepSingleByChannel(Hit, SafeLoc, RayTarget, FQuat::Identity, TraceChannel, SphereShape, SphereParams);
#if ENABLE_DRAW_DEBUG
			if (World->TimeSince(LastDrawDebugTime) < 1.f)
			{
				DrawDebugSphere(World, SafeLoc, SphereShape.Sphere.Radius, 8, bHit? FColor::Red : FColor::Green);
				DrawDebugSphere(World, bHit ? Hit.Location : RayTarget, SphereShape.Sphere.Radius, 8, bHit ? FColor::Red : FColor::Green);
				DrawDebugLine(World, SafeLoc, bHit ? Hit.Location : RayTarget, FColor::Red);
			}
#endif

			Feeler.FramesUntilNextTrace = Feeler.TraceInterval;

			const AActor* HitActor = Hit.GetActor();

			if (bHit && HitActor)
			{
				bool bIgnoreHit = false;

				if (HitActor->ActorHasTag(ZodiacCameraMode_ThirdPerson_Statics::NAME_IgnoreCameraCollision))
				{
					bIgnoreHit = true;
					SphereParams.AddIgnoredActor(HitActor);
				}

				// Ignore CameraBlockingVolume hits that occur in front of the ViewTarget.
				if (!bIgnoreHit && HitActor->IsA<ACameraBlockingVolume>())
				{
					const FVector ViewTargetForwardXY = ViewTarget.GetActorForwardVector().GetSafeNormal2D();
					const FVector ViewTargetLocation = ViewTarget.GetActorLocation();
					const FVector HitOffset = Hit.Location - ViewTargetLocation;
					const FVector HitDirectionXY = HitOffset.GetSafeNormal2D();
					const float DotHitDirection = FVector::DotProduct(ViewTargetForwardXY, HitDirectionXY);
					if (DotHitDirection > 0.0f)
					{
						bIgnoreHit = true;
						// Ignore this CameraBlockingVolume on the remaining sweeps.
						SphereParams.AddIgnoredActor(HitActor);
					}
					else
					{
#if ENABLE_DRAW_DEBUG
						DebugActorsHitDuringCameraPenetration.AddUnique(TObjectPtr<const AActor>(HitActor));
#endif
					}
				}
				
				if (!bIgnoreHit)
				{
					float const Weight = Cast<APawn>(Hit.GetActor()) ? Feeler.PawnWeight : Feeler.WorldWeight;
					float NewBlockPct = Hit.Time;
					NewBlockPct += (1.f - NewBlockPct) * (1.f - Weight);

					// Recompute blocked pct taking into account pushout distance.
					NewBlockPct = ((Hit.Location - SafeLoc).Size() - CollisionPushOutDistance) / (RayTarget - SafeLoc).Size();
					DistBlockedPctThisFrame = FMath::Min(NewBlockPct, DistBlockedPctThisFrame);

					// This feeler got a hit, so do another trace next frame
					Feeler.FramesUntilNextTrace = 0;

#if ENABLE_DRAW_DEBUG
					DebugActorsHitDuringCameraPenetration.AddUnique(TObjectPtr<const AActor>(HitActor));
#endif
				}
			}

			if (RayIdx == 0)
			{
				// don't interpolate toward this one, snap to it
				// assumes ray 0 is the center/main ray 
				HardBlockedPct = DistBlockedPctThisFrame;
			}
			else
			{
				SoftBlockedPct = DistBlockedPctThisFrame;
			}
		}
		else
		{
			--Feeler.FramesUntilNextTrace;
		}
	}

	if (bResetInterpolation)
	{
		DistBlockedPct = DistBlockedPctThisFrame;
	}
	else if (DistBlockedPct < DistBlockedPctThisFrame)
	{
		// interpolate smoothly out
		if (PenetrationBlendOutTime > DeltaTime)
		{
			DistBlockedPct = DistBlockedPct + DeltaTime / PenetrationBlendOutTime * (DistBlockedPctThisFrame - DistBlockedPct);
		}
		else
		{
			DistBlockedPct = DistBlockedPctThisFrame;
		}
	}
	else
	{
		if (DistBlockedPct > HardBlockedPct)
		{
			DistBlockedPct = HardBlockedPct;
		}
		else if (DistBlockedPct > SoftBlockedPct)
		{
			// interpolate smoothly in
			if (PenetrationBlendInTime > DeltaTime)
			{
				DistBlockedPct = DistBlockedPct - DeltaTime / PenetrationBlendInTime * (DistBlockedPct - SoftBlockedPct);
			}
			else
			{
				DistBlockedPct = SoftBlockedPct;
			}
		}
	}

	DistBlockedPct = FMath::Clamp<float>(DistBlockedPct, 0.f, 1.f);
	if (DistBlockedPct < (1.f - ZERO_ANIMWEIGHT_THRESH))
	{
		CameraLoc = SafeLoc + (CameraLoc - SafeLoc) * DistBlockedPct;
	}
}

bool UZodiacCameraMode_ThirdPerson::CheckCloseContact()
{
	if (!bHandleCloseContact)
	{
		return false;
	}

	UWorld* World = GetWorld();
	AActor* TargetActor = GetTargetActor();

	// Set up collision query parameters
	FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(CameraCloseContact), false, nullptr);
	SphereParams.AddIgnoredActor(TargetActor);
	SphereParams.AddIgnoredActors(TargetActor->Children);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(0);
	ECollisionChannel TraceChannel = ECC_Pawn;
	
	FVector EyeLoc;
	FRotator EyeRot;
	TargetActor->GetActorEyesViewPoint(EyeLoc, EyeRot);
	
	FVector BaseRayForwardVector = TargetActor->GetActorForwardVector();
	FRotationMatrix BaseRayMatrix(BaseRayForwardVector.Rotation());
	FVector BaseRayLocalUp, BaseRayLocalFwd, BaseRayLocalRight;
	BaseRayMatrix.GetScaledAxes(BaseRayLocalFwd, BaseRayLocalRight, BaseRayLocalUp);
	
	for (auto& Feeler : CloseContactFeelers)
	{
		if (Feeler.FramesUntilNextTrace <= 0)
		{
			Feeler.FramesUntilNextTrace = Feeler.TraceInterval;
			SphereShape.Sphere.Radius = Feeler.Extent;
			
			// calc ray target
			FVector RotatedRayDirection = BaseRayForwardVector.RotateAngleAxis(Feeler.AdjustmentRot.Yaw, BaseRayLocalUp);
			RotatedRayDirection = RotatedRayDirection.RotateAngleAxis(Feeler.AdjustmentRot.Pitch, BaseRayLocalRight);
			FVector OverlapCenter = EyeLoc + RotatedRayDirection * Feeler.ContactDistance;

			TArray<FOverlapResult> OverlapResults;
			bool bHasEnemyHit = false;
			if (bool bHit = World->OverlapMultiByChannel(OverlapResults, OverlapCenter, FQuat(), TraceChannel, SphereShape, SphereParams))
			{
				for (auto& Result : OverlapResults)
				{
					if (Result.bBlockingHit)
					{
						if (AZodiacMonster* Monster = Cast<AZodiacMonster>(Result.GetActor()))
						{
							Feeler.FramesUntilNextTrace = 0;
							bHasEnemyHit = true;
							break;
						}
					}
				}
			}
			
#if ENABLE_DRAW_DEBUG
			if (World->TimeSince(LastDrawDebugTime) < 1.f)
			{
				DrawDebugSphere(World, OverlapCenter, Feeler.Extent, 8, bHasEnemyHit ? FColor::Purple : FColor::White);	
			}
#endif
			if (bHasEnemyHit)
			{
				return true;
			}
		}
		else
		{
			--Feeler.FramesUntilNextTrace;
		}
	}

	return false;
}

void UZodiacCameraMode_ThirdPerson::SetCloseContactOffset(const FVector& InBlendStartOffset, const FVector& InBlendEndOffset, float Weight)
{
	const float InvExponent = (CloseContactBlendExponent > 0.0f) ? (1.0f / CloseContactBlendExponent) : 1.0f;

	switch (CloseContactBlendFunction)
	{
	case EZodiacCameraModeBlendFunction::Linear:
		TargetOffset = FMath::Lerp(InBlendStartOffset, InBlendEndOffset, Weight);
		break;

	case EZodiacCameraModeBlendFunction::EaseIn:
		TargetOffset = FMath::InterpEaseIn(InBlendStartOffset, InBlendEndOffset, Weight, InvExponent);
		break;

	case EZodiacCameraModeBlendFunction::EaseOut:
		TargetOffset = FMath::InterpEaseOut(InBlendStartOffset, InBlendEndOffset, Weight, InvExponent);
		break;

	case EZodiacCameraModeBlendFunction::EaseInOut:
		TargetOffset = FMath::InterpEaseInOut(InBlendStartOffset, InBlendEndOffset, Weight, InvExponent);
		break;

	default:
		checkf(false, TEXT("SetCloseContactOffset: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

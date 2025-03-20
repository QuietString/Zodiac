// Copyright Epic Games, Inc. All Rights Reserved.
// the.quiet.string@gmail.com

#pragma once

#include "ZodiacCameraMode.h"
#include "Curves/CurveFloat.h"
#include "ZodiacPenetrationAvoidanceFeeler.h"
#include "DrawDebugHelpers.h"
#include "ZodiacCloseContactFeeler.h"
#include "ZodiacCameraMode_ThirdPerson.generated.h"

class UCurveVector;

/**
 * UZodiacCameraMode_ThirdPerson
 *
 *	A basic third person camera mode.
 */
UCLASS(Abstract, Blueprintable)
class UZodiacCameraMode_ThirdPerson : public UZodiacCameraMode
{
	GENERATED_BODY()

public:
	UZodiacCameraMode_ThirdPerson();

protected:

	virtual void UpdateView(float DeltaTime) override;
	
	void UpdateTargetOffsetCurve(float DeltaTime, FRotator PivotRotation);
	void UpdatePreventPenetration(float DeltaTime);
	void PreventCameraPenetration(class AActor const& ViewTarget, FVector const& SafeLoc, FVector& CameraLoc, float const& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly);

	inline bool CheckCloseContact();
	inline void SetCloseContactOffset(const FVector& InBlendStartOffset, const FVector& InBlendEndOffset, float Weight);
	
	virtual void DrawDebug(UCanvas* Canvas) const override;

protected:
	
	// Curve that defines local-space offsets from the target using the view pitch to evaluate the curve.
	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
	TObjectPtr<const UCurveVector> TargetOffsetCurve;

	/** If true, does collision checks to move the camera to make aim easier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision|Close Contact")
	bool bHandleCloseContact = true;

	UPROPERTY(EditAnywhere, Category = "Collision|Close Contact")
	float CloseContactBlendInTime = 0.3f;
	
	UPROPERTY(EditAnywhere, Category = "Collision|Close Contact")
	float CloseContactBlendOutTime = 0.3f;
	
	// Curve that defines local-space offsets from the target applied when there is close contact at the front of the target
	UPROPERTY(EditDefaultsOnly, Category = "Collision|Close Contact")
	TObjectPtr<const UCurveVector> CloseContactOffsetCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Collision|Close Contact")
	EZodiacCameraModeBlendFunction CloseContactBlendFunction;

	UPROPERTY(EditDefaultsOnly, Category = "Collision|Close Contact")
	float CloseContactBlendExponent = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = "Collision|Close Contact")
	TArray<FZodiacCloseContactFeeler> CloseContactFeelers;
	
	// Penetration prevention
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	float PenetrationBlendInTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	float PenetrationBlendOutTime = 0.15f;

	/** If true, does collision checks to keep the camera out of the world. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	bool bPreventPenetration = true;
	
	/** If true, try to detect nearby walls and move the camera in anticipation.  Helps prevent popping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	bool bDoPredictiveAvoidance = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CollisionPushOutDistance = 2.f;

	/** When the camera's distance is pushed into this percentage of its full distance due to penetration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float ReportPenetrationPercent = 0.f;

	/**
	 * These are the feeler rays that are used to find where to place the camera.
	 * Index: 0  : This is the normal feeler we use to prevent collisions.
	 * Index: 1+ : These feelers are used if you bDoPredictiveAvoidance=true, to scan for potential impacts if the player
	 *             were to rotate towards that direction and primitively collide the camera so that it pulls in before
	 *             impacting the occluder.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<FZodiacPenetrationAvoidanceFeeler> PenetrationAvoidanceFeelers;
	
	UPROPERTY(Transient)
	float AimLineToDesiredPosBlockedPct;

	UPROPERTY(Transient)
	TArray<TObjectPtr<const AActor>> DebugActorsHitDuringCameraPenetration;

private:
	UPROPERTY(Transient)
	FVector TargetOffset;

	bool bIsBlending = false;
	float BlendElapsedTime = 0.0f;
	float BlendDuration = 0.0f;
	FVector BlendStartOffset;
	FVector BlendEndOffset;
	bool bPreviousCloseContact = false;

#if ENABLE_DRAW_DEBUG
	mutable float LastDrawDebugTime = -MAX_FLT;
#endif
};

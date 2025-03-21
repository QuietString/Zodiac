// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/World.h"
#include "GameplayTagContainer.h"

#include "ZodiacCameraMode.generated.h"

class AActor;
class UCanvas;
class UZodiacCameraComponent;

/**
 *	Blend function used for transitioning between camera modes.
 */
UENUM(BlueprintType)
enum class EZodiacCameraModeBlendFunction : uint8
{
	// Does a simple linear interpolation.
	Linear,

	// Immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by the exponent.
	EaseIn,

	// Smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by the exponent.
	EaseOut,

	// Smoothly accelerates and decelerates.  Ease amount controlled by the exponent.
	EaseInOut,

	COUNT	UMETA(Hidden)
};

struct FZodiacCameraShakeScalingParams
{
	FZodiacCameraShakeScalingParams()
		: MovementSpeed(0.f)
		, MovementAcceleration(0.f)
	{}

	FZodiacCameraShakeScalingParams(const float MovementSpeed, const float MovementAcceleration)
		: MovementSpeed(MovementSpeed)
		, MovementAcceleration(MovementAcceleration)
	{}
	
	float MovementSpeed;
	float MovementAcceleration;
};

/**
 *	View data produced by the camera mode that is used to blend camera modes.
 */
struct FZodiacCameraModeView
{
public:

	FZodiacCameraModeView();

	void Blend(const FZodiacCameraModeView& Other, float OtherWeight);

public:

	FVector Location;
	FRotator Rotation;
	FRotator ControlRotation;
	float FieldOfView;
};

UENUM(BlueprintType)
enum class EZodiacCameraShakeScalingMethod : uint8
{
	NoScaling,
	MovementSpeed,
	Acceleration
};

USTRUCT(BlueprintType)
struct FZodiacCameraShakeScalingConfig
{
	GENERATED_BODY()

	FZodiacCameraShakeScalingConfig()
		: Method(EZodiacCameraShakeScalingMethod::NoScaling)
		, BaseAmount(0)
	{}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EZodiacCameraShakeScalingMethod Method;

	// Amount of the method needed to apply full scale to this camera shake.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Method != EZodiacCameraShakeScalingMethod::NoScaling"))
	float BaseAmount;
};

/**
 *	Base class for all camera modes.
 */
UCLASS(Abstract, NotBlueprintable)
class ZODIAC_API UZodiacCameraMode : public UObject
{
	GENERATED_BODY()

public:

	UZodiacCameraMode();

	UZodiacCameraComponent* GetZodiacCameraComponent() const;

	virtual UWorld* GetWorld() const override;

	AActor* GetTargetActor() const;

	const FZodiacCameraModeView& GetCameraModeView() const { return View; }

	// Called when this camera mode is activated on the camera mode stack.
	virtual void OnActivation(){}

	// Called when this camera mode is deactivated on the camera mode stack.
	virtual void OnDeactivation();

	void UpdateCameraMode(float DeltaTime);
	void UpdateCameraShake();
	
	float GetBlendTime() const { return BlendTime; }
	float GetBlendWeight() const { return BlendWeight; }
	void SetBlendWeight(float Weight);

	FGameplayTag GetCameraTypeTag() const
	{
		return CameraTypeTag;
	}

	bool GetHasCloseContact() const { return bHasCloseContact; }

	virtual void DrawDebug(UCanvas* Canvas) const;

protected:

	virtual FVector GetPivotLocation() const;
	virtual FRotator GetPivotRotation() const;

	virtual void UpdateView(float DeltaTime);
	virtual void UpdateBlending(float DeltaTime);

protected:
	// A tag that can be queried by gameplay code that cares when a kind of camera mode is active
	// without having to ask about a specific mode (e.g., when aiming downsights to get more accuracy)
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	FGameplayTag CameraTypeTag;

	// View output produced by the camera mode.
	FZodiacCameraModeView View;

	// The horizontal field of view (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "5.0", UIMax = "170", ClampMin = "5.0", ClampMax = "170.0"))
	float FieldOfView;

	// Minimum view pitch (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMin;

	// Maximum view pitch (in degrees).
	UPROPERTY(EditDefaultsOnly, Category = "View", Meta = (UIMin = "-89.9", UIMax = "89.9", ClampMin = "-89.9", ClampMax = "89.9"))
	float ViewPitchMax;

	// How long it takes to blend in this mode.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendTime;

	// Function used for blending.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	EZodiacCameraModeBlendFunction BlendFunction;

	// Exponent used by blend functions to control the shape of the curve.
	UPROPERTY(EditDefaultsOnly, Category = "Blending")
	float BlendExponent;

	// Linear blend alpha used to determine the blend weight.
	float BlendAlpha;

	// Blend weight calculated using the blend alpha and function.
	float BlendWeight;
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Camera Shake")
	bool bPlayCameraShake;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shake", meta = (ClampMin = "0.0"))
	int32 UpdateTickInterval = 10;

private:
	UPROPERTY(Transient)
	TObjectPtr<UCameraShakeBase> CameraShake;
	
	UPROPERTY(Transient)
	int32 FramesUntilNextShakeUpdate = 0;
	
protected:
	// Looping Camera Shake. Single Instance only.
	UPROPERTY(EditAnywhere, Category = "Camera Shake", meta = (EditCondition = "bPlayCameraShake"))
	TSubclassOf<UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditAnywhere, Category = "Camera Shake", meta = (EditCondition = "bPlayCameraShake"))
	FZodiacCameraShakeScalingConfig LocationScalingConfig;
	
	UPROPERTY(EditAnywhere, Category = "Camera Shake", meta = (EditCondition = "bPlayCameraShake"))
	FZodiacCameraShakeScalingConfig RotationScalingConfig;
	
protected:
	/** If true, skips all interpolation and puts camera in ideal location.  Automatically set to false next frame. */
	UPROPERTY(Transient)
	uint32 bResetInterpolation:1;

	UPROPERTY(Transient)
	bool bHasCloseContact = false;
};


/**
 * UZodiacCameraModeStack
 *
 *	Stack used for blending camera modes.
 */
UCLASS()
class UZodiacCameraModeStack : public UObject
{
	GENERATED_BODY()

public:

	UZodiacCameraModeStack();

	void ActivateStack();
	void DeactivateStack();

	bool IsStackActivate() const { return bIsActive; }

	void PushCameraMode(TSubclassOf<UZodiacCameraMode> CameraModeClass);

	bool EvaluateStack(float DeltaTime, FZodiacCameraModeView& OutCameraModeView);

	void DrawDebug(UCanvas* Canvas) const;

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

	UZodiacCameraMode* GetTopCameraMode() const;
	
protected:

	UZodiacCameraMode* GetCameraModeInstance(TSubclassOf<UZodiacCameraMode> CameraModeClass);

	void UpdateStack(float DeltaTime);
	void BlendStack(FZodiacCameraModeView& OutCameraModeView) const;

protected:
	bool bIsActive;

	UPROPERTY()
	TArray<TObjectPtr<UZodiacCameraMode>> CameraModeInstances;

	UPROPERTY()
	TArray<TObjectPtr<UZodiacCameraMode>> CameraModeStack;
};

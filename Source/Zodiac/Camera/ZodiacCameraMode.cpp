// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacCameraMode.h"

#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "Character/Host/ZodiacHostCharacter.h"
#include "ZodiacCameraComponent.h"
#include "ZodiacPlayerCameraManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Shakes/PerlinNoiseCameraShakePattern.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacCameraMode)


//////////////////////////////////////////////////////////////////////////
// FZodiacCameraModeView
//////////////////////////////////////////////////////////////////////////
FZodiacCameraModeView::FZodiacCameraModeView()
	: Location(ForceInit)
	, Rotation(ForceInit)
	, ControlRotation(ForceInit)
	, FieldOfView(ZODIAC_CAMERA_DEFAULT_FOV)
{
}

void FZodiacCameraModeView::Blend(const FZodiacCameraModeView& Other, float OtherWeight)
{
	if (OtherWeight <= 0.0f)
	{
		return;
	}
	else if (OtherWeight >= 1.0f)
	{
		*this = Other;
		return;
	}

	Location = FMath::Lerp(Location, Other.Location, OtherWeight);

	const FRotator DeltaRotation = (Other.Rotation - Rotation).GetNormalized();
	Rotation = Rotation + (OtherWeight * DeltaRotation);

	const FRotator DeltaControlRotation = (Other.ControlRotation - ControlRotation).GetNormalized();
	ControlRotation = ControlRotation + (OtherWeight * DeltaControlRotation);

	FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
}


//////////////////////////////////////////////////////////////////////////
// UZodiacCameraMode
//////////////////////////////////////////////////////////////////////////
UZodiacCameraMode::UZodiacCameraMode()
{
	FieldOfView = ZODIAC_CAMERA_DEFAULT_FOV;
	ViewPitchMin = ZODIAC_CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = ZODIAC_CAMERA_DEFAULT_PITCH_MAX;

	BlendTime = 0.5f;
	BlendFunction = EZodiacCameraModeBlendFunction::EaseOut;
	BlendExponent = 4.0f;
	BlendAlpha = 1.0f;
	BlendWeight = 1.0f;
}

UZodiacCameraComponent* UZodiacCameraMode::GetZodiacCameraComponent() const
{
	return CastChecked<UZodiacCameraComponent>(GetOuter());
}

UWorld* UZodiacCameraMode::GetWorld() const
{
	return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld();
}

AActor* UZodiacCameraMode::GetTargetActor() const
{
	const UZodiacCameraComponent* ZodiacCameraComponent = GetZodiacCameraComponent();

	return ZodiacCameraComponent->GetTargetActor();
}

void UZodiacCameraMode::OnDeactivation()
{
	if (CameraShake)
	{
		if (UZodiacCameraComponent* ZodiacCameraComponent = Cast<UZodiacCameraComponent>(GetZodiacCameraComponent()))
		{
			if (APlayerCameraManager* PCM = ZodiacCameraComponent->GetPlayerCameraManager())
			{
				PCM->StopCameraShake(CameraShake);
			}
		}

		CameraShake = nullptr;
	}
}

FVector UZodiacCameraMode::GetPivotLocation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		// Height adjustments for characters to account for crouching.
		if (const AZodiacHostCharacter* TargetCharacter = Cast<AZodiacHostCharacter>(TargetPawn))
		{
			const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
			check(TargetCharacterCDO);

			const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
			check(CapsuleComp);

			const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
			check(CapsuleCompCDO);

			const UZodiacCameraComponent* ZodiacCameraComponent = GetZodiacCameraComponent();
			check(ZodiacCameraComponent);
			
			const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
			const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
			const float HeightAdjustment = (DefaultHalfHeight - ActualHalfHeight) + TargetCharacterCDO->BaseEyeHeight;
			const FVector TranslationOffset = ZodiacCameraComponent->TranslationOffset;
			
			return TargetCharacter->GetActorLocation() + (FVector::UpVector * HeightAdjustment) + TranslationOffset;
		}

		return TargetPawn->GetPawnViewLocation();
	}

	return TargetActor->GetActorLocation();
}

FRotator UZodiacCameraMode::GetPivotRotation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		return TargetPawn->GetViewRotation();
	}

	return TargetActor->GetActorRotation();
}

void UZodiacCameraMode::UpdateCameraMode(float DeltaTime)
{
	UpdateView(DeltaTime);
	UpdateBlending(DeltaTime);
	UpdateCameraShake();
}

void UZodiacCameraMode::UpdateView(float DeltaTime)
{
	FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = GetPivotRotation();

	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}

void UZodiacCameraMode::SetBlendWeight(float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

	// Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend function.
	const float InvExponent = (BlendExponent > 0.0f) ? (1.0f / BlendExponent) : 1.0f;

	switch (BlendFunction)
	{
	case EZodiacCameraModeBlendFunction::Linear:
		BlendAlpha = BlendWeight;
		break;

	case EZodiacCameraModeBlendFunction::EaseIn:
		BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case EZodiacCameraModeBlendFunction::EaseOut:
		BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	case EZodiacCameraModeBlendFunction::EaseInOut:
		BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;

	default:
		checkf(false, TEXT("SetBlendWeight: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

void UZodiacCameraMode::UpdateBlending(float DeltaTime)
{
	if (BlendTime > 0.0f)
	{
		BlendAlpha += (DeltaTime / BlendTime);
		BlendAlpha = FMath::Min(BlendAlpha, 1.0f);
	}
	else
	{
		BlendAlpha = 1.0f;
	}

	const float Exponent = (BlendExponent > 0.0f) ? BlendExponent : 1.0f;

	switch (BlendFunction)
	{
	case EZodiacCameraModeBlendFunction::Linear:
		BlendWeight = BlendAlpha;
		break;

	case EZodiacCameraModeBlendFunction::EaseIn:
		BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case EZodiacCameraModeBlendFunction::EaseOut:
		BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	case EZodiacCameraModeBlendFunction::EaseInOut:
		BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;

	default:
		checkf(false, TEXT("UpdateBlending: Invalid BlendFunction [%d]\n"), (uint8)BlendFunction);
		break;
	}
}

void UZodiacCameraMode::UpdateCameraShake()
{
	if (!bPlayCameraShake)
	{
		return;
	}
	
	UZodiacCameraComponent* ZodiacCameraComponent = GetZodiacCameraComponent();
	if (!ZodiacCameraComponent)
	{
		return;
	}

	APlayerCameraManager* PCM = ZodiacCameraComponent->GetPlayerCameraManager();
	if (!PCM)
	{
		return;
	}

	if (FramesUntilNextShakeUpdate > 0)
	{
		FramesUntilNextShakeUpdate--;
		return;
	}

	if (!CameraShake)
	{
		CameraShake = PCM->StartCameraShake(CameraShakeClass, 1.f);	
	}

	if (UPerlinNoiseCameraShakePattern* Pattern = Cast<UPerlinNoiseCameraShakePattern>(CameraShake->GetRootShakePattern()))
	{
		float Speed = 0.0f;
		float Acceleration = 0.f;
		
		if (LocationScalingConfig.Method == EZodiacCameraShakeScalingMethod::MovementSpeed || RotationScalingConfig.Method == EZodiacCameraShakeScalingMethod::MovementSpeed)
		{
			if (ACharacter* Character = Cast<ACharacter>(GetTargetActor()))
			{
				Speed = Character->GetVelocity().Size();
			}
		}
		
		if (LocationScalingConfig.Method == EZodiacCameraShakeScalingMethod::Acceleration || RotationScalingConfig.Method == EZodiacCameraShakeScalingMethod::Acceleration)
		{
			if (ACharacter* Character = Cast<ACharacter>(GetTargetActor()))
			{
				if (UCharacterMovementComponent* CharacterMovementComponent = Character->GetCharacterMovement())
				{
					Acceleration = CharacterMovementComponent->GetCurrentAcceleration().Size();
				}
			}
		}

		float LocationMultiplier = 1.f;
		float LocationBaseAmount = LocationScalingConfig.BaseAmount;
		if (LocationBaseAmount > 0.f)
		{
			LocationMultiplier = LocationScalingConfig.Method == EZodiacCameraShakeScalingMethod::MovementSpeed ? Speed / LocationBaseAmount :
				LocationScalingConfig.Method == EZodiacCameraShakeScalingMethod::Acceleration ? Acceleration / LocationBaseAmount : 1.f;
		}
		LocationMultiplier = FMath::Max(LocationMultiplier, 0.0f);
		
		Pattern->LocationAmplitudeMultiplier = LocationMultiplier;
		Pattern->LocationFrequencyMultiplier = LocationMultiplier;

		float RotationMultiplier = 1.f;
		float RotationBaseAmount = RotationScalingConfig.BaseAmount;
		if (RotationBaseAmount > 0.f)
		{
			RotationMultiplier = RotationScalingConfig.Method == EZodiacCameraShakeScalingMethod::MovementSpeed ? Speed / RotationBaseAmount :
				RotationScalingConfig.Method == EZodiacCameraShakeScalingMethod::Acceleration ? Acceleration / RotationBaseAmount : 1.f;	
		}
		RotationMultiplier = FMath::Max(RotationMultiplier, 0.0f);
		
		Pattern->RotationAmplitudeMultiplier = RotationMultiplier;
		Pattern->RotationAmplitudeMultiplier = RotationMultiplier;
	}

	FramesUntilNextShakeUpdate = UpdateTickInterval;
}

void UZodiacCameraMode::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("      ZodiacCameraMode: %s (%f)"), *GetName(), BlendWeight));
}


//////////////////////////////////////////////////////////////////////////
// UZodiacCameraModeStack
//////////////////////////////////////////////////////////////////////////
UZodiacCameraModeStack::UZodiacCameraModeStack()
{
	bIsActive = true;
}

void UZodiacCameraModeStack::ActivateStack()
{
	if (!bIsActive)
	{
		bIsActive = true;

		// Notify camera modes that they are being activated.
		for (UZodiacCameraMode* CameraMode : CameraModeStack)
		{
			check(CameraMode);
			CameraMode->OnActivation();
		}
	}
}

void UZodiacCameraModeStack::DeactivateStack()
{
	if (bIsActive)
	{
		bIsActive = false;

		// Notify camera modes that they are being deactivated.
		for (UZodiacCameraMode* CameraMode : CameraModeStack)
		{
			check(CameraMode);
			CameraMode->OnDeactivation();
		}
	}
}

void UZodiacCameraModeStack::PushCameraMode(TSubclassOf<UZodiacCameraMode> CameraModeClass)
{
	if (!CameraModeClass)
	{
		return;
	}

	UZodiacCameraMode* CameraMode = GetCameraModeInstance(CameraModeClass);
	check(CameraMode);

	int32 StackSize = CameraModeStack.Num();

	if ((StackSize > 0) && (CameraModeStack[0] == CameraMode))
	{
		// Already top of stack.
		return;
	}

	// See if it's already in the stack and remove it.
	// Figure out how much it was contributing to the stack.
	int32 ExistingStackIndex = INDEX_NONE;
	float ExistingStackContribution = 1.0f;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		if (CameraModeStack[StackIndex] == CameraMode)
		{
			ExistingStackIndex = StackIndex;
			ExistingStackContribution *= CameraMode->GetBlendWeight();
			break;
		}
		else
		{
			ExistingStackContribution *= (1.0f - CameraModeStack[StackIndex]->GetBlendWeight());
		}
	}

	if (ExistingStackIndex != INDEX_NONE)
	{
		CameraModeStack.RemoveAt(ExistingStackIndex);
		StackSize--;
	}
	else
	{
		ExistingStackContribution = 0.0f;
	}

	// Decide what initial weight to start with.
	const bool bShouldBlend = ((CameraMode->GetBlendTime() > 0.0f) && (StackSize > 0));
	const float BlendWeight = (bShouldBlend ? ExistingStackContribution : 1.0f);

	CameraMode->SetBlendWeight(BlendWeight);

	// Add new entry to top of stack.
	CameraModeStack.Insert(CameraMode, 0);

	// Make sure stack bottom is always weighted 100%.
	CameraModeStack.Last()->SetBlendWeight(1.0f);

	// Let the camera mode know if it's being added to the stack.
	if (ExistingStackIndex == INDEX_NONE)
	{
		CameraMode->OnActivation();
	}
}

bool UZodiacCameraModeStack::EvaluateStack(float DeltaTime, FZodiacCameraModeView& OutCameraModeView)
{
	if (!bIsActive)
	{
		return false;
	}

	UpdateStack(DeltaTime);
	BlendStack(OutCameraModeView);

	return true;
}

UZodiacCameraMode* UZodiacCameraModeStack::GetCameraModeInstance(TSubclassOf<UZodiacCameraMode> CameraModeClass)
{
	check(CameraModeClass);

	// First see if we already created one.
	for (UZodiacCameraMode* CameraMode : CameraModeInstances)
	{
		if ((CameraMode != nullptr) && (CameraMode->GetClass() == CameraModeClass))
		{
			return CameraMode;
		}
	}

	// Not found, so we need to create it.
	UZodiacCameraMode* NewCameraMode = NewObject<UZodiacCameraMode>(GetOuter(), CameraModeClass, NAME_None, RF_NoFlags);
	check(NewCameraMode);

	CameraModeInstances.Add(NewCameraMode);

	return NewCameraMode;
}

void UZodiacCameraModeStack::UpdateStack(float DeltaTime)
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0)
	{
		return;
	}

	int32 RemoveCount = 0;
	int32 RemoveIndex = INDEX_NONE;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		UZodiacCameraMode* CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		CameraMode->UpdateCameraMode(DeltaTime);

		if (CameraMode->GetBlendWeight() >= 1.0f)
		{
			// Everything below this mode is now irrelevant and can be removed.
			RemoveIndex = (StackIndex + 1);
			RemoveCount = (StackSize - RemoveIndex);
			break;
		}
	}

	if (RemoveCount > 0)
	{
		// Let the camera modes know they being removed from the stack.
		for (int32 StackIndex = RemoveIndex; StackIndex < StackSize; ++StackIndex)
		{
			UZodiacCameraMode* CameraMode = CameraModeStack[StackIndex];
			check(CameraMode);

			CameraMode->OnDeactivation();
		}

		CameraModeStack.RemoveAt(RemoveIndex, RemoveCount);
	}
}

void UZodiacCameraModeStack::BlendStack(FZodiacCameraModeView& OutCameraModeView) const
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0)
	{
		return;
	}

	// Start at the bottom and blend up the stack
	const UZodiacCameraMode* CameraMode = CameraModeStack[StackSize - 1];
	check(CameraMode);

	OutCameraModeView = CameraMode->GetCameraModeView();

	for (int32 StackIndex = (StackSize - 2); StackIndex >= 0; --StackIndex)
	{
		CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		OutCameraModeView.Blend(CameraMode->GetCameraModeView(), CameraMode->GetBlendWeight());
	}
}

void UZodiacCameraModeStack::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString(TEXT("   --- Camera Modes (Begin) ---")));

	for (const UZodiacCameraMode* CameraMode : CameraModeStack)
	{
		check(CameraMode);
		CameraMode->DrawDebug(Canvas);
	}

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   --- Camera Modes (End) ---")));
}

void UZodiacCameraModeStack::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	if (CameraModeStack.Num() == 0)
	{
		OutWeightOfTopLayer = 1.0f;
		OutTagOfTopLayer = FGameplayTag();
	}
	else
	{
		UZodiacCameraMode* TopEntry = CameraModeStack.Last();
		check(TopEntry);
		OutWeightOfTopLayer = TopEntry->GetBlendWeight();
		OutTagOfTopLayer = TopEntry->GetCameraTypeTag();
	}
}

UZodiacCameraMode* UZodiacCameraModeStack::GetTopCameraMode() const
{
	if (!CameraModeStack.IsEmpty())
	{
		if (UZodiacCameraMode* TopEntry = CameraModeStack.Last())
		{
			return TopEntry;	
		}	
	}

	return nullptr;
}

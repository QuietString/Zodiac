// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacCameraComponent.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "ZodiacCameraMode.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacCameraComponent)


UZodiacCameraComponent::UZodiacCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bApplyTranslationOffset = true;
	CameraModeStack = nullptr;
	FieldOfViewOffset = 0.0f;
}

APlayerCameraManager* UZodiacCameraComponent::GetPlayerCameraManager()
{
	if (APawn* TargetPawn = Cast<APawn>(GetTargetActor()))
	{
		if (APlayerController* PC = TargetPawn->GetController<APlayerController>())
		{
			return PC->PlayerCameraManager;
		}
	}
	
	return nullptr;
}

void UZodiacCameraComponent::OnRegister()
{
	Super::OnRegister();

	if (!CameraModeStack)
	{
		CameraModeStack = NewObject<UZodiacCameraModeStack>(this);
		check(CameraModeStack);
	}
}

void UZodiacCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	check(CameraModeStack);

	UpdateCameraModes();

	FZodiacCameraModeView CameraModeView;
	CameraModeStack->EvaluateStack(DeltaTime, CameraModeView);

	// Keep player controller in sync with the latest view.
	if (APawn* TargetPawn = Cast<APawn>(GetTargetActor()))
	{
		if (APlayerController* PC = TargetPawn->GetController<APlayerController>())
		{
			PC->SetControlRotation(CameraModeView.ControlRotation);
		}
	}

	ApplyTranslationOffset(DeltaTime, CameraModeView);
	
	// Apply any offset that was added to the field of view.
	CameraModeView.FieldOfView += FieldOfViewOffset;
	FieldOfViewOffset = 0.0f;

	// Keep camera component in sync with the latest view.
	SetWorldLocationAndRotation(CameraModeView.Location, CameraModeView.Rotation);
	FieldOfView = CameraModeView.FieldOfView;
	
	// Fill in desired view.
	DesiredView.Location = CameraModeView.Location;
	DesiredView.Rotation = CameraModeView.Rotation;
	DesiredView.FOV = CameraModeView.FieldOfView;
	DesiredView.OrthoWidth = OrthoWidth;
	DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
	DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;
	DesiredView.AspectRatio = AspectRatio;
	DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
	DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	DesiredView.ProjectionMode = ProjectionMode;

	// See if the CameraActor wants to override the PostProcess settings used.
	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f)
	{
		DesiredView.PostProcessSettings = PostProcessSettings;
	}

	HandleCloseContact();
	
	if (IsXRHeadTrackedCamera())
	{
		// In XR much of the camera behavior above is irrellevant, but the post process settings are not.
		Super::GetCameraView(DeltaTime, DesiredView);
	}

	LastViewLocation = DesiredView.Location;
}

void UZodiacCameraComponent::ApplyTranslationOffset(float DeltaTime, FZodiacCameraModeView& CameraModeView)
{
	if (UpdateCameraTranslationOffsetDelegate.IsBound())
	{
		FVector TranslationOffset = UpdateCameraTranslationOffsetDelegate.Execute();
		if (bIgnoreZAxis)
		{
			TranslationOffset.Z = 0.f;
		}

		FVector TargetLocation = CameraModeView.Location + TranslationOffset;
		
		if (TranslationOffsetInterpSpeed > 0.0f)
		{
			CameraModeView.Location = FMath::VInterpTo(LastViewLocation, TargetLocation, DeltaTime, TranslationOffsetInterpSpeed);	
		}
		else
		{
			CameraModeView.Location = TargetLocation;
		}
	}
}

void UZodiacCameraComponent::HandleCloseContact()
{
	bHasCloseContact = CheckHasCloseTarget();
	
	if (bHasCloseContact && !bPreviousCloseContact)
	{
		OnCloseContactStarted.ExecuteIfBound();
	}

	if (!bHasCloseContact && bPreviousCloseContact)
	{
		OnCloseContactFinished.ExecuteIfBound();
	}

	bPreviousCloseContact = bHasCloseContact;
}

bool UZodiacCameraComponent::CheckHasCloseTarget()
{
	if (UZodiacCameraMode* TopCameraMode = CameraModeStack->GetTopCameraMode())
	{
		return TopCameraMode->GetHasCloseContact();
	}

	return false;
}

void UZodiacCameraComponent::UpdateCameraModes()
{
	check(CameraModeStack);

	if (CameraModeStack->IsStackActivate())
	{
		if (DetermineCameraModeDelegate.IsBound())
		{
			if (const TSubclassOf<UZodiacCameraMode> CameraMode = DetermineCameraModeDelegate.Execute())
			{
				CameraModeStack->PushCameraMode(CameraMode);
			}
		}
	}
}

FVector UZodiacCameraComponent::GetHeroOffset()
{
	if (AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetTargetActor()))
	{
		return HostCharacter->GetHeroEyeLocationOffset();
	}

	return  FVector(0);
}

void UZodiacCameraComponent::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("ZodiacCameraComponent: %s"), *GetNameSafe(GetTargetActor())));

	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   Location: %s"), *GetComponentLocation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   Rotation: %s"), *GetComponentRotation().ToCompactString()));
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   FOV: %f"), FieldOfView));

	check(CameraModeStack);
	CameraModeStack->DrawDebug(Canvas);
}

void UZodiacCameraComponent::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	check(CameraModeStack);
	CameraModeStack->GetBlendInfo(OUT OutWeightOfTopLayer, OUT OutTagOfTopLayer);
}
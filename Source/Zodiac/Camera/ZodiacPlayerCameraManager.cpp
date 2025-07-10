// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacPlayerCameraManager.h"

#include "Async/TaskGraphInterfaces.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "ZodiacCameraComponent.h"
#include "ZodiacUICameraManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacPlayerCameraManager)

class FDebugDisplayInfo;

static FName UICameraComponentName(TEXT("UICamera"));

AZodiacPlayerCameraManager::AZodiacPlayerCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultFOV = ZODIAC_CAMERA_DEFAULT_FOV;
	ViewPitchMin = ZODIAC_CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = ZODIAC_CAMERA_DEFAULT_PITCH_MAX;

	UICamera = CreateDefaultSubobject<UZodiacUICameraManagerComponent>(UICameraComponentName);
}

UZodiacUICameraManagerComponent* AZodiacPlayerCameraManager::GetUICameraComponent() const
{
	return UICamera;
}

void AZodiacPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	// If the UI Camera is looking at something, let it have priority.
	if (UICamera->NeedsToUpdateViewTarget())
	{
		Super::UpdateViewTarget(OutVT, DeltaTime);
		UICamera->UpdateViewTarget(OutVT, DeltaTime);
		return;
	}

	Super::UpdateViewTarget(OutVT, DeltaTime);
}

void AZodiacPlayerCameraManager::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("ZodiacPlayerCameraManager: %s"), *GetNameSafe(this)));

	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	const APawn* Pawn = (PCOwner ? PCOwner->GetPawn() : nullptr);

	if (const UZodiacCameraComponent* CameraComponent = UZodiacCameraComponent::FindCameraComponent(Pawn))
	{
		CameraComponent->DrawDebug(Canvas);
	}
}


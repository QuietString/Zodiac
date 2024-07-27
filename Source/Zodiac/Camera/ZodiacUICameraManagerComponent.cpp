// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacUICameraManagerComponent.h"

#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "ZodiacPlayerCameraManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacUICameraManagerComponent)

class AActor;
class FDebugDisplayInfo;

UZodiacUICameraManagerComponent* UZodiacUICameraManagerComponent::GetComponent(APlayerController* PC)
{
	if (PC != nullptr)
	{
		if (AZodiacPlayerCameraManager* PCCamera = Cast<AZodiacPlayerCameraManager>(PC->PlayerCameraManager))
		{
			return PCCamera->GetUICameraComponent();
		}
	}

	return nullptr;
}

UZodiacUICameraManagerComponent::UZodiacUICameraManagerComponent()
{
	bWantsInitializeComponent = true;

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		// Register "showdebug" hook.
		if (!IsRunningDedicatedServer())
		{
			AHUD::OnShowDebugInfo.AddUObject(this, &ThisClass::OnShowDebugInfo);
		}
	}
}

void UZodiacUICameraManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UZodiacUICameraManagerComponent::SetViewTarget(AActor* InViewTarget, FViewTargetTransitionParams TransitionParams)
{
	TGuardValue<bool> UpdatingViewTargetGuard(bUpdatingViewTarget, true);

	ViewTarget = InViewTarget;
	CastChecked<AZodiacPlayerCameraManager>(GetOwner())->SetViewTarget(ViewTarget, TransitionParams);
}

bool UZodiacUICameraManagerComponent::NeedsToUpdateViewTarget() const
{
	return false;
}

void UZodiacUICameraManagerComponent::UpdateViewTarget(struct FTViewTarget& OutVT, float DeltaTime)
{
}

void UZodiacUICameraManagerComponent::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
}

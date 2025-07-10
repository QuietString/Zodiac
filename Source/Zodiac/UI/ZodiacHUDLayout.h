// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ZodiacActivatableWidget.h"

#include "ZodiacHUDLayout.generated.h"

class UCommonActivatableWidget;
class UObject;

/**
 *	Widget used to lay out the player's HUD (typically specified by an Add Widgets action in the experience)
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Meta = (DisplayName = "Zodiac HUD Layout", Category = "Zodiac|HUD"))
class UZodiacHUDLayout : public UZodiacActivatableWidget
{
	GENERATED_BODY()

public:
	UZodiacHUDLayout(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;

protected:
	void HandleEscapeAction();

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;
};

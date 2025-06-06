// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"

#include "ZodiacActivatableWidget.generated.h"

struct FUIInputConfig;

UENUM(BlueprintType)
enum class EZodiacWidgetInputMode : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};

// An activatable widget that automatically drives the desired input config when activated
UCLASS(Abstract, Blueprintable)
class UZodiacActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UZodiacActivatableWidget(const FObjectInitializer& ObjectInitializer);

	void DeactivateOnAnimFinished(FWidgetAnimationState& WidgetAnimationState);
	virtual void NativeOnDeactivated() override;
public:
	
	//~UCommonActivatableWidget interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UCommonActivatableWidget interface

#if WITH_EDITOR
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif
	
protected:
	/** The desired input mode to use while this UI is activated, for example do you want key presses to still reach the game/player controller? */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EZodiacWidgetInputMode InputConfig = EZodiacWidgetInputMode::Default;

	/** The desired mouse behavior when the game gets input. */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> BoundAnim_OnActivated;
};

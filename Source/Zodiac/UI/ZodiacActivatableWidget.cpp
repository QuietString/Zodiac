// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacActivatableWidget.h"

#include "Animation/UMGSequencePlayer.h"
#include "Editor/WidgetCompilerLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacActivatableWidget)

#define LOCTEXT_NAMESPACE "Zodiac"

UZodiacActivatableWidget::UZodiacActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZodiacActivatableWidget::DeactivateOnAnimFinished(FWidgetAnimationState& WidgetAnimationState)
{
	Super::NativeOnDeactivated();
}

void UZodiacActivatableWidget::NativeOnDeactivated()
{
	if (BoundAnim_OnActivated)
	{
		PlayAnimationReverse(BoundAnim_OnActivated).GetAnimationState()->GetOnWidgetAnimationFinished().AddUObject(this, &ThisClass::DeactivateOnAnimFinished);
	}
	else
	{
		Super::NativeOnDeactivated();
	}
}

TOptional<FUIInputConfig> UZodiacActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case EZodiacWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case EZodiacWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case EZodiacWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case EZodiacWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}

#if WITH_EDITOR

void UZodiacActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UZodiacActivatableWidget, BP_GetDesiredFocusTarget)))
	{
		if (GetParentNativeClass(GetClass()) == UZodiacActivatableWidget::StaticClass())
		{
			CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen."));
		}
		else
		{
			CompileLog.Note(LOCTEXT("ValidateGetDesiredFocusTarget_Note", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.  If it was implemented in the native base class you can ignore this message."));
		}
	}
}

#endif

#undef LOCTEXT_NAMESPACE

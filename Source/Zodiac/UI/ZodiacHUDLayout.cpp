// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacHUDLayout.h"

#include "CommonInputBaseTypes.h"
#include "CommonUIExtensions.h"
#include "Input/CommonUIInputTypes.h"
#include "NativeGameplayTags.h"
#include "ZodiacGameplayTags.h"
#include "UI/ZodiacActivatableWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHUDLayout)


UZodiacHUDLayout::UZodiacHUDLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZodiacHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(ZodiacGameplayTags::UI_Action_Escape), false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction)));
}

void UZodiacHUDLayout::HandleEscapeAction()
{
	if (ensure(!EscapeMenuClass.IsNull()))
	{
		UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(GetOwningLocalPlayer(), ZodiacGameplayTags::UI_Layer_Menu, EscapeMenuClass);
	}
}
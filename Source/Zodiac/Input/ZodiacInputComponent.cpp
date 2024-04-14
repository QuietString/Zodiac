// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Player/ZodiacLocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacInputComponent)

class UZodiacInputConfig;

UZodiacInputComponent::UZodiacInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UZodiacInputComponent::AddInputMappings(const UZodiacInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to add something from your input config if required
}

void UZodiacInputComponent::RemoveInputMappings(const UZodiacInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to remove input mappings that you may have added above
}

void UZodiacInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}

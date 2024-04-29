// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacInputTagMapping.h"

#include "ZodiacLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacInputTagMapping)

UZodiacInputTagMapping::UZodiacInputTagMapping(const FObjectInitializer& ObjectInitializer)
{
}

const UInputAction* UZodiacInputTagMapping::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FZodiacInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogZodiac, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* UZodiacInputTagMapping::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FZodiacInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogZodiac, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

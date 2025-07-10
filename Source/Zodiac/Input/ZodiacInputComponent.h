// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EnhancedInputComponent.h"
#include "ZodiacInputTagMapping.h"

#include "ZodiacInputComponent.generated.h"

class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UObject;


USTRUCT()
struct FZodiacInputConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UInputMappingContext>> MappingContexts;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputMappingContext> InAirContext;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UZodiacInputTagMapping> TagMapping;

public:
	bool IsValid()
	{
		return MappingContexts.Num() > 0 && TagMapping;
	}
};


/**
 *	Component used to manage input mappings and bindings using an input config data asset.
 */
UCLASS(Config = Input)
class UZodiacInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	UZodiacInputComponent(const FObjectInitializer& ObjectInitializer);

	void AddInputMappings(const UZodiacInputTagMapping* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	void RemoveInputMappings(const UZodiacInputTagMapping* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	template<class UserClass, typename FuncType>
	void BindNativeAction(const UZodiacInputTagMapping* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UZodiacInputTagMapping* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

	void RemoveBinds(TArray<uint32>& BindHandles);

protected:

	bool IsPawnLocallyControlled();
};


template<class UserClass, typename FuncType>
void UZodiacInputComponent::BindNativeAction(const UZodiacInputTagMapping* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	if (IsPawnLocallyControlled())
	{
		check(InputConfig);
		if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
		{
			BindAction(IA, TriggerEvent, Object, Func);
		}	
	}
}

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UZodiacInputComponent::BindAbilityActions(const UZodiacInputTagMapping* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	if (IsPawnLocallyControlled())
	{
		check(InputConfig);

		for (const FZodiacInputAction& Action : InputConfig->AbilityInputActions)
		{
			if (Action.InputAction && Action.InputTag.IsValid())
			{
				if (PressedFunc)
				{
					BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
				}

				if (ReleasedFunc)
				{
					BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
				}
			}
		}
	}
}

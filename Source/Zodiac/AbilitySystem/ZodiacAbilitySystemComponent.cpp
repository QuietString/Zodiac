// the.quiet.string@gmail.com


#include "ZodiacAbilitySystemComponent.h"

#include "Abilities/ZodiacGameplayAbility.h"


UZodiacAbilitySystemComponent::UZodiacAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void UZodiacAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	UE_LOG(LogTemp, Warning, TEXT("ability input tag: %s"), *InputTag.GetTagName().ToString());


	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UZodiacAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void UZodiacAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// @TODO: if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))
	// {
	// 	ClearAbilityInput();
	// 	return;
	// }

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	// Process all abilities that activate when the input is held.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UZodiacGameplayAbility* ZodiacAbilityCDO = CastChecked<UZodiacGameplayAbility>(AbilitySpec->Ability);

				AbilitiesToActivate.AddUnique(AbilitySpec->Handle);

				// @TODO: if (ZodiacAbilityCDO->GetActivationPolicy() == EZodiacAbilityActivationPolicy::WhileInputActive)
				// {
				// 	AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				// }
			}
		}
	}

	// Process all abilities that had their input pressed this frame.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const UZodiacGameplayAbility* ZodiacAbilityCDO = CastChecked<UZodiacGameplayAbility>(AbilitySpec->Ability);

					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);

					// @TODO: if (ZodiacAbilityCDO->GetActivationPolicy() == EZodiacAbilityActivationPolicy::OnInputTriggered)
					// {
					// 	AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					// }
				}
			}
		}
	}

	//
	// Try to activate all the abilities that are from presses and holds.
	// We do it all at once so that held inputs don't activate the ability
	// and then also send a input event to the ability because of the press.
	//
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	// Process all abilities that had their input released this frame.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	// Clear the cached ability handles.
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UZodiacAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle,
	UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);
	
}

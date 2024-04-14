// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ZodiacAbilitySystemComponent.generated.h"

struct FGameplayTag;
struct FGameplayAbilitySpec;

/**
 * UZodiacAbilitySystemComponent
 *
 *	Base ability system component class used by this project.
 */
UCLASS()
class ZODIAC_API UZodiacAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UZodiacAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
public:


protected:
	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
};

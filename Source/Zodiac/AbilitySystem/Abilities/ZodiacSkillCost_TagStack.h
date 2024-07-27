// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "ZodiacAbilityCost.h"
#include "ScalableFloat.h"

#include "ZodiacSkillCost_TagStack.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpecHandle;

class UZodiacGameplayAbility;
class UObject;
struct FGameplayAbilityActorInfo;

/**
 * Represents a cost that requires expending a quantity of a tag stack
 */
UCLASS(meta=(DisplayName="Tag Stack"))
class UZodiacSkillCost_TagStack : public UZodiacAbilityCost
{
	GENERATED_BODY()

public:
	UZodiacSkillCost_TagStack();

	//~UZodiacAbilityCost interface
	virtual bool CheckCost(const UZodiacGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const UZodiacGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~End of UZodiacAbilityCost interface

	float GetQuantity() const { return Quantity.GetValueAtLevel(0); }
	FGameplayTag TagToSpend() const { return Tag; }
	
protected:
	/** How much of the tag to spend */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FScalableFloat Quantity;

	/** Which tag to spend some of */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FGameplayTag Tag;

	/** Which tag to send back as a response if this cost cannot be applied */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FGameplayTag FailureTag;
};

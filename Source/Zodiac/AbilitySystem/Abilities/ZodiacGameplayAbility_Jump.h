// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ZodiacGameplayAbility.h"

#include "ZodiacGameplayAbility_Jump.generated.h"

class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
 *	Gameplay ability used for character jumping.
 */
UCLASS(Abstract)
class UZodiacGameplayAbility_Jump : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:

	UZodiacGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
protected:

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnInputRelease(float TimeHeld);
	
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	void CharacterJumpStart();

	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	void CharacterJumpStop();

private:
	bool bHasJumped;
};

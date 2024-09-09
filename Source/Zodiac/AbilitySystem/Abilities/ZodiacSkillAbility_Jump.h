// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ZodiacGameplayAbility.h"

#include "ZodiacSkillAbility_Jump.generated.h"

class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayTagContainer;


/**
 *	Gameplay ability used for character jumping.
 */
UCLASS(Abstract)
class UZodiacSkillAbility_Jump : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:

	UZodiacSkillAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnJustLanded(FGameplayEventData Payload);
	
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	void CharacterJumpStart();

	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	void CharacterJumpStop();
};
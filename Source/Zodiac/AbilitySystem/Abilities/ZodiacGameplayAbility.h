// Copyright Epic Games, Inc. All Rights Reserved.
// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "System/GameplayTagStack.h"
#include "ZodiacGameplayAbility.generated.h"

class AZodiacHostCharacter;
class AZodiacCharacter;
class UZodiacAbilitySystemComponent;
class UZodiacAbilityCost;
class AZodiacPlayerController;
class UZodiacCameraMode;
class AZodiacHostCharacter;

/**
 *	Defines how an ability is meant to activate.
 */
UENUM(BlueprintType)
enum class EZodiacAbilityActivationPolicy : uint8
{
	// Try to activate the ability when the input is triggered.
	OnInputTriggered,

	// Continually try to activate the ability while the input is active.
	WhileInputActive,

	// Try to activate the ability when an avatar is assigned.
	OnSpawn
};


/**
 *	Defines how an ability activates in relation to other abilities.
 */
UENUM(BlueprintType)
enum class EZodiacAbilityActivationGroup : uint8
{
	// Ability runs independently of all other abilities.
	Independent,

	// Ability is canceled and replaced by other exclusive abilities.
	Exclusive_Replaceable,

	// Ability blocks all other exclusive abilities from activating.
	Exclusive_Blocking,

	MAX	UMETA(Hidden)
};

/** Failure reason that can be used to play an animation montage when a failure occurs */
USTRUCT(BlueprintType)
struct FZodiacAbilityMontageFailureMessage
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	// All the reasons why this ability has failed
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> FailureMontage = nullptr;
};

/**
 *	The base gameplay ability class used by this project.
 */
UCLASS(Abstract, HideCategories = Input, Meta = (ShortTooltip = "The base gameplay ability class used by this project."))
class ZODIAC_API UZodiacGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UZodiacGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	AZodiacCharacter* GetZodiacCharacterFromActorInfo() const;
	
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	AController* GetControllerFromActorInfo() const;

	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	EZodiacAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	EZodiacAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;
	
	// Sets the ability's camera mode.
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	virtual void SetCameraMode(TSubclassOf<UZodiacCameraMode> CameraMode);

	// Clears the ability's camera mode.  Automatically called if needed when the ability ends.
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	virtual void ClearCameraMode();
	
	void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
	{
		NativeOnAbilityFailedToActivate(FailedReason);
		ScriptOnAbilityFailedToActivate(FailedReason);
	}

protected:
	
	// Called when the ability fails to activate
	virtual void NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	// Called when the ability fails to activate
	UFUNCTION(BlueprintImplementableEvent)
	void ScriptOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

protected:
	
	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	EZodiacAbilityActivationPolicy ActivationPolicy;

	// Defines the relationship between this ability activating and other abilities activating.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	EZodiacAbilityActivationGroup ActivationGroup;

	// Map of failure tags to simple error messages
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;

	// Map of failure tags to anim montages that should be played with them
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;

	// Current camera mode set by the ability.
	TSubclassOf<UZodiacCameraMode> ActiveCameraMode;
};

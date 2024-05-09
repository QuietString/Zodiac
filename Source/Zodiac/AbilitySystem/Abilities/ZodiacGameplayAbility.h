// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "System/GameplayTagStack.h"
#include "ZodiacGameplayAbility.generated.h"

class UZodiacAbilityCost;
class UZodiacHeroComponent;
class AZodiacPlayerController;
class UZodiacCameraMode;
class AZodiacPlayerCharacter;

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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	AZodiacPlayerController* GetZodiacPlayerControllerFromActorInfo() const;
	
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	AZodiacPlayerCharacter* GetZodiacCharacterFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	UZodiacHeroComponent* GetCurrentHeroComponent() const;

	UFUNCTION(BlueprintNativeEvent)
	FName GetCurrentAbilitySocket(const uint8 ComboIndex);

	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="TagStack")
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= "TagStack")
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);
	
	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category= "TagStack")
	int32 GetStatTagStackCount(FGameplayTag Tag) const;
	
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION(BlueprintCallable)
	float GetCooldownDuration() const { return CooldownDuration.GetValueAtLevel(GetAbilityLevel()); }
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual  void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	EZodiacAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	EZodiacAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;
	
	// Sets the ability's camera mode.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	void SetCameraMode(TSubclassOf<UZodiacCameraMode> CameraMode);

	// Clears the ability's camera mode.  Automatically called if needed when the ability ends.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	void ClearCameraMode();
	
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

	// Called on CommitExecute.
	void SendCooldownMessage();
protected:

	// Skill identifier tag for effects (sockets and etc)
	UPROPERTY(BlueprintReadOnly, Category = "Zodiac|Ability")
	FGameplayTag SkillTag;
	
	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	EZodiacAbilityActivationPolicy ActivationPolicy;

	// Defines the relationship between this ability activating and other abilities activating.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
	EZodiacAbilityActivationGroup ActivationGroup;

	// Additional costs that must be paid to activate this ability
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Costs")
	TArray<TObjectPtr<UZodiacAbilityCost>> AdditionalCosts;

	// initial amount of tag stack to give. e.g, ammo
	UPROPERTY(EditDefaultsOnly, Category = "Costs")
	TMap<FGameplayTag, int32> InitialTagStack;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Costs")
	FScalableFloat CooldownDuration;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Costs")
	FGameplayTagContainer CooldownTags;
	
	// Map of failure tags to simple error messages
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;

	// Map of failure tags to anim montages that should be played with them
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;

	// Current camera mode set by the ability.
	TSubclassOf<UZodiacCameraMode> ActiveCameraMode;

private:

	// Tag container for additional costs. e.g, ammo
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;
	
	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;
};

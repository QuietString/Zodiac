// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "Abilities/ZodiacGameplayAbility.h"
#include "ZodiacAbilitySystemComponent.generated.h"

class UZodiacHeroAbility;
struct FGameplayTag;
struct FGameplayAbilitySpec;

ZODIAC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);

/**
 *	Base ability system component class used by this project.
 */
UCLASS()
class ZODIAC_API UZodiacAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UZodiacAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface
	
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	typedef TFunctionRef<bool(const UZodiacGameplayAbility* ZodiacAbility, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);

	void CancelInputActivatedAbilities(bool bReplicateCancelAbility);

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();
	
	bool IsActivationGroupBlocked(EZodiacAbilityActivationGroup Group) const;
	void AddAbilityToActivationGroup(EZodiacAbilityActivationGroup Group, UZodiacGameplayAbility* ZodiacAbility);
	void RemoveAbilityFromActivationGroup(EZodiacAbilityActivationGroup Group, UZodiacGameplayAbility* ZodiacAbility);
	void CancelActivationGroupAbilities(EZodiacAbilityActivationGroup Group, UZodiacGameplayAbility* IgnoreZodiacAbility, bool bReplicateCancelAbility);

	// Uses a gameplay effect to add the specified dynamic granted tag.
	void AddDynamicTagGameplayEffect(const FGameplayTag& Tag);

	// Removes all active instances of the gameplay effect that was used to add the specified dynamic granted tag.
	void RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag);
	
	/** Gets the ability target data associated with the given ability handle and activation info */
	void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, const FGameplayAbilityActivationInfo& ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

	UFUNCTION(BlueprintCallable)
	bool SetActiveGameplayEffectDuration(FActiveGameplayEffectHandle Handle, float InDuration);

	UFUNCTION(BlueprintPure)
	bool IsLocallyPredicted() const;

#if WITH_EDITOR
	virtual int32 HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload) override;
#endif
	
protected:
	void TryActivateAbilitiesOnSpawn();
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;

	/** Notify client that an ability failed to activate */
	UFUNCTION(Client, Unreliable)
	void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	auto HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) -> void;

	// Add local gameplay tag. Added tag will be removed even when the ability added this tag ended before the scheduled time.
	UFUNCTION(BlueprintCallable)
	void AddLooseGameplayTagForDuration(FGameplayTag Tag, float Duration, FTimerHandle& RemoveHandle);
	
protected:
	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	// Number of abilities running in each activation group.
	int32 ActivationGroupCounts[(uint8)EZodiacAbilityActivationGroup::MAX];
};

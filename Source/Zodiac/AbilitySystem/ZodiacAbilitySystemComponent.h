// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "Abilities/ZodiacGameplayAbility.h"
#include "ZodiacAbilitySystemComponent.generated.h"

class UZodiacSkillAbility;
struct FGameplayTag;
struct FGameplayAbilitySpec;

ZODIAC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);

UCLASS(BlueprintType)
class USkillMuzzleSocketData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FName MuzzleSocket;
};

USTRUCT(BlueprintType)
struct FGameplayCueReadyData
{
	GENERATED_BODY()

public:
	void SetGCNParameters(const FGameplayCueParameters& NewParameters)
	{
		const UObject* RegisteredSourceObject = GCNParameters.SourceObject.Get();
		GCNParameters = NewParameters;

		if (RegisteredSourceObject)
		{
			GCNParameters.SourceObject = RegisteredSourceObject;
		}
		
		bGCNParameterReady = true;
	}

	void SetMuzzleSocket(const USkillMuzzleSocketData* MuzzleSocketData)
	{
		GCNParameters.SourceObject = MuzzleSocketData;
		bMuzzleSocketReady = true;
	}

	void SetGameplayTagCue(FGameplayTag NewGameplayTagCue)
	{
		GameplayCueTag = NewGameplayTagCue;
		bGameplayCueTagSet = true;
	}

	bool IsReady()
	{
		return bGCNParameterReady && bMuzzleSocketReady && bGameplayCueTagSet;
	}

	void Reset()
	{
		GCNParameters = FGameplayCueParameters();
		bGCNParameterReady = false;
		bMuzzleSocketReady = false;
		bGameplayCueTagSet = false;
	}

protected:

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag GameplayCueTag;
	
	UPROPERTY(BlueprintReadWrite)
	FGameplayCueParameters GCNParameters;

	bool bGameplayCueTagSet = false;
	bool bGCNParameterReady = false;
	bool bMuzzleSocketReady = false;

	friend UZodiacAbilitySystemComponent;
};

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

	/** Gets the ability target data associated with the given ability handle and activation info */
	void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, const FGameplayAbilityActivationInfo& ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

	UFUNCTION(BlueprintCallable)
	void SetMuzzleSocketData(FName NewMuzzleSocketName);

	UFUNCTION(BlueprintCallable)
	void CheckAndExecuteGameplayCue();

protected:
	
	void TryActivateAbilitiesOnSpawn();

	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;

	/** Notify client that an ability failed to activate */
	UFUNCTION(Client, Unreliable)
	void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

public:
	UPROPERTY(BlueprintReadWrite)
	FGameplayCueReadyData GameplayCueReadyData;
	
protected:

	// For GameplayCue effects.
	UPROPERTY(Transient)
	USkillMuzzleSocketData* MuzzleSocketData;
	
	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	// Number of abilities running in each activation group.
	int32 ActivationGroupCounts[(uint8)EZodiacAbilityActivationGroup::MAX];
};

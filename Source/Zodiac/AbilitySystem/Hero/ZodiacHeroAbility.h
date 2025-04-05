// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "Hero/ZodiacHeroAbilitySlotActor.h"
#include "ZodiacHeroAbility.generated.h"

class AZodiacHeroAbilitySlotActor;
class UNiagaraSystem;
class UZodiacHeroAbilitySlot;
class AZodiacHeroCharacter;

/**
 * Gameplay ability belongs to a hero character
 */
UCLASS(Abstract)
class ZODIAC_API UZodiacHeroAbility : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAbilitySystemComponent* GetHostAbilitySystemComponent() const;

	virtual AZodiacHostCharacter* GetZodiacHostCharacterFromActorInfo() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AZodiacPlayerController* GetHostPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AZodiacHeroCharacter* GetHeroActorFromActorInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AZodiacHeroCharacter* GetHeroCharacterFromActorInfo() const;
	
	UFUNCTION(BlueprintCallable)
	UZodiacHeroAbilitySlot* GetAssociatedSlot() const;

	void AddSlotAsSourceObject(FGameplayEffectContextHandle& EffectContext);
	
	UFUNCTION(BlueprintCallable)
	FGameplayEffectContextHandle AddSourceSlotToEffectContext(FGameplayEffectContextHandle EffectContext);
	
	template<typename T>
	T* GetAssociatedSlot() const
	{
		if (UZodiacHeroAbilitySlot* Slot = GetAssociatedSlot())
		{
			if (T* TypedSlot = Cast<T>(Slot))
			{
				return TypedSlot;
			}
		}

		return nullptr;
	}
	
	UFUNCTION(BlueprintCallable)
	float GetCooldownDuration() const { return CooldownDuration.GetValueAtLevel(GetAbilityLevel()); }

	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	bool ShouldUseInitialCost() const { return bHasInitialCost && !bHasAppliedInitialCost; }
	bool GetHasInitialCost() const { return bHasInitialCost; }
	bool GetHasCheckedInitialCost() const { return bHasCheckedInitialCost; }
	float GetCostToApply() const { return bHasInitialCost ? (bHasAppliedInitialCost ? CostAmount : InitialCostAmount) : CostAmount; }
	float GetCostAmount() const { return CostAmount; }
	
	FVector GetWeaponLocation() const;

protected:
	UFUNCTION(BlueprintCallable)
	void ApplySlotReticle();

	UFUNCTION(BlueprintCallable)
	void ClearSlotReticle();

	UFUNCTION(BlueprintCallable)
	void ClearCameraModeAfterDuration(float Duration, FTimerHandle& RemoveHandle);
	
	UFUNCTION(BlueprintCallable)
	void SetTargetAttachComponent(USceneComponent* InAttachComponent) { TargetAttachComponent = InAttachComponent; }

	UFUNCTION(BlueprintCallable)
	void ClearTargetAttachComponent() { TargetAttachComponent = nullptr; }

	UFUNCTION(BlueprintCallable)
	AZodiacHeroAbilitySlotActor* GetCurrentSocketSourceActor() const;
	
	UFUNCTION(BlueprintCallable)
	void AdvanceCombo();
	
	UFUNCTION(BlueprintCallable)
	void ResetCombo() { ComboIndex = 0; }
	
	UFUNCTION(BlueprintCallable)
	void ChargeUltimate();

	UFUNCTION(BlueprintCallable)
	void ExecuteGameplayCueOnHost(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParams);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cooldowns")
	FScalableFloat CooldownDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cooldowns")
	FGameplayTagContainer CooldownTags;

	UPROPERTY(BlueprintReadOnly, Category = "FX")
	uint8 ComboIndex = 0;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = Costs)
	TArray<TObjectPtr<UZodiacAbilityCost>> AdditionalCosts;

	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer ActivationOwnedTagsHost;

	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer ActivationRequiredTagsHost;

	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer ActivationBlockedTagsHost;

	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer BlockAbilitiesWithTag_Host;
	
	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer CancelAbilitiesWithTag_Host;
	
	// List of actors that will be used for specific combo fire. Each actor has a socket for firing. These actors should be included in ActorsToSpawn in HeroAbilitySlotDefinition
	UPROPERTY(EditAnywhere, Category = "FX")
	TArray<TSubclassOf<AZodiacHeroAbilitySlotActor>> SocketSourceClasses;

	UPROPERTY(EditAnywhere, Category = "FX")
	TObjectPtr<USceneComponent> TargetAttachComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate")
	TSubclassOf<UGameplayEffect> ChargeUltimateEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ultimate")
	FScalableFloat UltimateChargeAmount;
	
	// use different amount of cost for initiation when ability consume cost continuously during activation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs")
	bool bHasInitialCost = false;

	// Not used. UZodiacGameplayAttribute::InitialApplyAmount is used instead.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs", meta = (EditCondition = "bHasInitialCost"))
	float InitialCostAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs")
	float CostAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs")
	TArray<uint8> ComboToIgnoreAdditionalCost;
	
private:
	UPROPERTY(Transient)
	mutable TMap<uint8, AZodiacHeroAbilitySlotActor*> CachedSocketSourceActors;
	
	// These are used for checking which amount of cost should be used in CheckCost() and ApplyCost()
	mutable bool bHasCheckedInitialCost = false;
	mutable bool bHasAppliedInitialCost = false;
	
	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;
};
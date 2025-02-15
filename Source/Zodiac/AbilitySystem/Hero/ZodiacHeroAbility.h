// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "ZodiacHeroAbility.generated.h"

class UNiagaraSystem;
class UZodiacHeroAbilitySlot;
class AZodiacHeroCharacter;

USTRUCT(BlueprintType)
struct FZodiacImpactParticles
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UParticleSystem> Default;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UParticleSystem> Character;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UParticleSystem> Concrete;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UParticleSystem> Glass;
};

UCLASS(BlueprintType, Const, DefaultToInstanced, EditInlineNew, DisplayName = "Socket Object", CollapseCategories)
class UZodiacAbilitySourceSocket : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> Trace_Niagara;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UParticleSystem> Trace_Particle;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FZodiacImpactParticles Impacts;
};

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

	FName GetCurrentComboSocket();
	
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
	void AdvanceCombo();
	
	UFUNCTION(BlueprintCallable)
	void ResetCombo() { ComboIndex = 0; }
	
	UFUNCTION(BlueprintCallable)
	void ChargeUltimate();
	
	UFUNCTION(BlueprintCallable)
	FVector GetSourceLocation() const;

	UFUNCTION(BlueprintCallable)
	UZodiacAbilitySourceSocket* GetSocket() const;

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

	UPROPERTY(EditAnywhere, Instanced, Category = "FX")
	TArray<UZodiacAbilitySourceSocket*> Sockets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate")
	TSubclassOf<UGameplayEffect> ChargeUltimateEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ultimate")
	FScalableFloat UltimateChargeAmount;
	
	// use different amount of cost for initiation when ability consume cost continuously during activation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs")
	bool bHasInitialCost = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs", meta = (EditCondition = "bHasInitialCost"))
	float InitialCostAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs")
	float CostAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs")
	TArray<uint8> ComboToIgnoreAdditionalCost;
	
private:
	// These are used for checking which amount of cost should be used in CheckCost() and ApplyCost()
	mutable bool bHasCheckedInitialCost = false;
	mutable bool bHasAppliedInitialCost = false;
	
	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;
};
// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "ZodiacSkillAbility.generated.h"

class UZodiacSkillAbilityCost;
class UZodiacSkillSlot;

USTRUCT(BlueprintType)
struct FZodiacCostData_Single
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	FScalableFloat CostGEAmount = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<UZodiacSkillAbilityCost>> AdditionalCosts;
};

USTRUCT(BlueprintType)
struct FZodiacSkillCostData
{
	GENERATED_BODY()

public:
	TArray<TObjectPtr<UZodiacSkillAbilityCost>> GetCurrentAdditionalCostData(const bool bIsFirstActivation) const
	{
		if (bUseMidActivationCost)
		{
			if (!bIsFirstActivation)
			{
				return MidActivationCostData.AdditionalCosts;	
			}
		}

		return  ActivationCostData.AdditionalCosts;
	}

	FScalableFloat GetCurrentCostGEAmount(const bool bIsFirstActivation) const
	{
		if (bUseMidActivationCost)
		{
			if (!bIsFirstActivation)
			{
				return MidActivationCostData.CostGEAmount;	
			}
		}

		return ActivationCostData.CostGEAmount;
	}

public:
	/**
	 * An additional cost that applied during activation.
	 * If true, ActivationCostAmount will be applied only once on activation.
	 */
	UPROPERTY(EditDefaultsOnly)
	bool bUseMidActivationCost = false;

	UPROPERTY(EditDefaultsOnly)
	FZodiacCostData_Single ActivationCostData;

	UPROPERTY(EditDefaultsOnly, meta=(EditCondition="bUseMidActivationCost"))
	FZodiacCostData_Single MidActivationCostData;
};

/**
 * Gameplay ability belongs to a hero character
 */
UCLASS()
class ZODIAC_API UZodiacSkillAbility : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	UZodiacSkillSlot* GetSkillSlot() const;
	
	UFUNCTION(BlueprintCallable)
	float GetCooldownDuration() const { return CooldownDuration.GetValueAtLevel(GetAbilityLevel()); }

	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	const FZodiacSkillCostData& GetCostEffectData() { return CostData;	}
	
	float GetCurrentGECostAmount() const;
	bool GetIsSubordinate() const { return bIsSubordinate; }
	TArray<TObjectPtr<UZodiacSkillAbilityCost>> GetCurrentAdditionalCosts();
	
protected:
	// Called on CommitExecute.
	void SendCooldownMessage();

	UFUNCTION(BlueprintCallable)
	void ChargeUltimate();

	UFUNCTION(BlueprintCallable)
	virtual void ProceedCombo() { ComboIndex = (ComboSockets.Num() > 0) ? (ComboIndex + 1) % ComboSockets.Num() : 0; }

	UFUNCTION(BlueprintCallable)
	virtual void ResetCombo() { ComboIndex = 0; }
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FScalableFloat SkillMultiplier = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate")
	TSubclassOf<UGameplayEffect> ChargeUltimateEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ultimate")
	FScalableFloat UltimateChargeAmount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Costs")
	FZodiacSkillCostData CostData;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldowns")
	FScalableFloat CooldownDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<FName> ComboSockets;

	// if this skill is subordinate skill of the other. if true, it won't display cost data.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	bool bIsSubordinate = false;
	
	UPROPERTY(BlueprintReadOnly)
	uint8 ComboIndex = 0;

private:
	bool bIsFirstActivation = false;
	
	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;
};
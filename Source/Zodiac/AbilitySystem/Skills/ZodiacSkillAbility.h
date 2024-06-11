// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "ZodiacSkillAbility.generated.h"

class UZodiacSkillSlot;

USTRUCT(BlueprintType)
struct FZodiacCostEffectData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FScalableFloat ActivationCostAmount = 0.0f;
	
	/**
	 * An additional cost that applied during activation.
	 * If true, ActivationCostAmount will be applied only once on activation.
	 */
	UPROPERTY(EditDefaultsOnly)
	bool bUseSeparateMidActivationCost = false;
	
	// A cost that will be used while mid-activation
	UPROPERTY(EditDefaultsOnly, meta=(EditContition="bUseSeparateMidActivationCost == true"))
	FScalableFloat MidActivationCostAmount = 0.0f;
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
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	float GetCurrentCostAmount() const;
	float GetInitialActivationCost() const;
	
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
	FZodiacCostEffectData CostData;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldowns")
	FScalableFloat CooldownDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<FName> ComboSockets;

	UPROPERTY(BlueprintReadOnly)
	uint8 ComboIndex = 0;
	
private:
	bool bIsFirstActivation = false;
	
	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;
};
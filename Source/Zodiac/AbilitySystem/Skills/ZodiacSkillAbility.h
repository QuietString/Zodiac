// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "ZodiacSkillAbility.generated.h"

class UZodiacSkillSlot;

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
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual  void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	float GetRequiredCostAmount() const;
	
protected:
	// Called on CommitExecute.
	void SendCooldownMessage();
	
	void ChargeUltimate();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FScalableFloat SkillMultiplier = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Ultimate")
	TSubclassOf<UGameplayEffect> ChargeUltimateEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Ultimate")
	FScalableFloat UltimateChargeAmount;

	// @TODO: for ultimate skill only for now. 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs")
	FScalableFloat RequiredCostAmount;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldowns")
	FScalableFloat CooldownDuration;

private:
	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;
	
};

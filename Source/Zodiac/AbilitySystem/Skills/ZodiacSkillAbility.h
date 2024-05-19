// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "ZodiacSkillAbility.generated.h"

/**
 * Gameplay ability belongs to a hero character
 */
UCLASS()
class ZODIAC_API UZodiacSkillAbility : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	float GetCooldownDuration() const { return CooldownDuration.GetValueAtLevel(GetAbilityLevel()); }

	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual  void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	// Called on CommitExecute.
	void SendCooldownMessage();
	
	void ChargeUltimate();

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Skill|Cost")
	FScalableFloat CooldownDuration;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SKill|Cost")
	FGameplayTagContainer CooldownTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Ultimate")
	TSubclassOf<UGameplayEffect> ChargeUltimateEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill|Ultimate")
	FScalableFloat UltimateChargeAmount;
	
private:
	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;
};

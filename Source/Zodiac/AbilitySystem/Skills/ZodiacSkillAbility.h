// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "ZodiacSkillAbility.generated.h"

class UZodiacSkillAbilityCost;
class UZodiacSkillSlot;

/**
 * Gameplay ability belongs to a hero character
 */
UCLASS(Abstract)
class ZODIAC_API UZodiacSkillAbility : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAbilitySystemComponent* GetHostAbilitySystemComponent() const;
	
	UFUNCTION(BlueprintCallable)
	UZodiacSkillSlot* GetSkillSlot() const;
	
	UFUNCTION(BlueprintCallable)
	float GetCooldownDuration() const { return CooldownDuration.GetValueAtLevel(GetAbilityLevel()); }

	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
	FVector GetFXSourceLocation() const;
	
protected:
	void ApplyAimingEffect();

	// Activate next part of this ability
	UFUNCTION(BlueprintCallable)
	void AdvanceComboIndex();

	UFUNCTION(BlueprintCallable)
	void ResetCombo() { ComboIndex = 0; }
	
protected:
	// Apply MOVE_Aiming movement mode when it's activated
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	bool bAimWhenActivated;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<UGameplayEffect> AimingEffect;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldowns")
	FScalableFloat CooldownDuration;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FX")
	TArray<FName> ComboSockets;
	
	UPROPERTY(BlueprintReadOnly, Category = "FX")
	uint8 ComboIndex = 0;

private:
	bool bIsFirstActivation = false;
	
	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;
};
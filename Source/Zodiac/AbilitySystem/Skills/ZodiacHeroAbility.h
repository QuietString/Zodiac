// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "ZodiacHeroAbility.generated.h"

class UZodiacHeroAbilitySlot;
class UZodiacHeroItemSlot;
class AZodiacHeroCharacter;
class UZodiacSkillInstance;

UCLASS(BlueprintType, Const, DefaultToInstanced, EditInlineNew, DisplayName = "Socket Object")
class UZodiacAbilitySourceSocket : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SocketName;
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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AZodiacPlayerController* GetHostPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AZodiacHeroCharacter* GetHeroActorFromActorInfo() const;
	
	UFUNCTION(BlueprintCallable)
	UZodiacHeroAbilitySlot* GetAssociatedSlot() const;
	
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
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	FVector GetWeaponLocation() const;
	
protected:
	void ApplyAimingEffect();

	UFUNCTION(BlueprintCallable)
	void AdvanceCombo();
	
	UFUNCTION(BlueprintCallable)
	void ResetCombo() { ComboIndex = 0; }

	UFUNCTION(BlueprintCallable)
	FVector GetSourceLocation() const;
	
	UZodiacAbilitySourceSocket* GetSocket() const;
protected:
	// Apply MOVE_Aiming movement mode when it's activated
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	bool bAimWhenActivated;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<UGameplayEffect> AimingEffect;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldowns")
	FScalableFloat CooldownDuration;

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

private:
	bool bIsFirstActivation = false;
	
	// Temp container that we will return the pointer to in GetCooldownTags().
	// This will be a union of our CooldownTags and the Cooldown GE's cooldown tags.
	UPROPERTY(Transient)
	FGameplayTagContainer TempCooldownTags;
};
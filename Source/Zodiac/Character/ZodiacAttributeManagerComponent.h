// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Components/GameFrameworkComponent.h"
#include "ZodiacAttributeManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FZodiacHealth_DeathEvent, AActor*, OwningActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FZodiacHealth_AttributeChanged, UZodiacAttributeManagerComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);

class UZodiacHealthSet;
class UAbilitySystemComponent;

/**
 *	Defines current state of death.
 */
UENUM(BlueprintType)
enum class EZodiacDeathState : uint8
{
	NotDead = 0,
	DeathStarted,
	DeathFinished
};

/**
 *	An actor component used to handle anything related to health.
 */
UCLASS()
class ZODIAC_API UZodiacAttributeManagerComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UZodiacAttributeManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Returns the health component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Zodiac|Health")
	static UZodiacAttributeManagerComponent* FindHealthComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UZodiacAttributeManagerComponent>() : nullptr); }

	UFUNCTION(BlueprintPure, Category = "Zodiac|Health")
	static UZodiacAttributeManagerComponent* FindMatchingHealthComponent(const AActor* Actor, const UAbilitySystemComponent* ASC)
	{
		TInlineComponentArray<UZodiacAttributeManagerComponent*> HealthComponents;
		if (Actor)
		{
			Actor->GetComponents<UZodiacAttributeManagerComponent>(HealthComponents);
			if (HealthComponents.Num() > 0)
			{
				for (auto& HealthComponent : HealthComponents)
				{
					if (HealthComponent->AbilitySystemComponent == ASC)
					{
						return HealthComponent;
					}
				}
			}
		}

		return nullptr;
	}

	void InitializeWithAbilitySystem(UZodiacAbilitySystemComponent* InASC);

	UFUNCTION(BlueprintCallable, Category = "Zodiac|Health")
	EZodiacDeathState GetDeathState() const { return DeathState; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Zodiac|Health", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsDeadOrDying() const { return (DeathState > EZodiacDeathState::NotDead); }

	void GetCurrentHealth(float& CurrentHealth, float& CurrentMaxHealth);
	
	// Begins the death sequence for the owner.
	virtual void StartDeath();

	// Ends the death sequence for the owner.
	virtual void FinishDeath();

public:

	// Delegate fired when the health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FZodiacHealth_AttributeChanged OnHealthChanged;

	// Delegate fired when the death sequence has started.
	UPROPERTY(BlueprintAssignable)
	FZodiacHealth_DeathEvent OnDeathStarted;

	// Delegate fired when the death sequence has finished.
	UPROPERTY(BlueprintAssignable)
	FZodiacHealth_DeathEvent OnDeathFinished;

	FSimpleMulticastDelegate OnComponentReady;
	
protected:
	
	void HandleHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);

	void HandleUltimateGaugeChanged(const FOnAttributeChangeData& OnAttributeChangeData);

	void SendUltimateChargeMessage(float NewUltimateGauge);
	
	void ClearGameplayTags();
	
	UFUNCTION()
	virtual void OnRep_DeathState(EZodiacDeathState OldDeathState);

private:

	void CheckReady();
	
public:
	
	// // Just for health visualization.
	// UPROPERTY(BlueprintReadOnly)
	// float MaxHealth = -1;
	//
	// UPROPERTY(BlueprintReadOnly)
	// float CurrentHealth = -1;
	
protected:

	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<const UZodiacHealthSet> HealthSet;

	// Replicated state used to handle dying.
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EZodiacDeathState DeathState;

private:
	
};

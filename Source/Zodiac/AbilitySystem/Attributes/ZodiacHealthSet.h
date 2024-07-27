// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacAttributeSet.h"
#include "ZodiacHealthSet.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacHealthSet : public UZodiacAttributeSet
{
	GENERATED_BODY()

public:
	UZodiacHealthSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	ATTRIBUTE_ACCESSORS(UZodiacHealthSet, Health);
	ATTRIBUTE_ACCESSORS(UZodiacHealthSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UZodiacHealthSet, Healing);
	ATTRIBUTE_ACCESSORS(UZodiacHealthSet, Damage);
	
	mutable FZodiacAttributeEvent OnHealthChanged;
	mutable FZodiacAttributeEvent OnMaxHealthChanged;
	mutable FZodiacAttributeEvent OnOutOfHealth;
	
protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

private:
	
	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData Health;

	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth;
	
	bool bOutOfHealth;

	// Store the health before any changes 
	float MaxHealthBeforeAttributeChange;
	float HealthBeforeAttributeChange;

	// Incoming healing. This is mapped directly to +Health
	UPROPERTY(BlueprintReadOnly, Category="Zodiac|Health", Meta=(AllowPrivateAccess=true))
	FGameplayAttributeData Healing;

	// Incoming damage. This is mapped directly to -Health
	UPROPERTY(BlueprintReadOnly, Category="Zodiac|Health", Meta=(HideFromModifiers, AllowPrivateAccess=true))
	FGameplayAttributeData Damage;
};

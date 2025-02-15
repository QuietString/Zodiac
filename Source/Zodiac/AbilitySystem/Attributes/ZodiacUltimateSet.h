// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attributes/ZodiacAttributeSet.h"
#include "ZodiacUltimateSet.generated.h"

UCLASS()
class ZODIAC_API UZodiacUltimateSet : public UZodiacAttributeSet
{
	GENERATED_BODY()

public:

	UZodiacUltimateSet();

	ATTRIBUTE_ACCESSORS(UZodiacUltimateSet, Ultimate);
	ATTRIBUTE_ACCESSORS(UZodiacUltimateSet, MaxUltimate);
	ATTRIBUTE_ACCESSORS(UZodiacUltimateSet, InitialCheckAmount);
	ATTRIBUTE_ACCESSORS(UZodiacUltimateSet, InitialApplyAmount);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_Ultimate(const FGameplayAttributeData& OldValue);

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:
	UPROPERTY(ReplicatedUsing=OnRep_Ultimate, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData Ultimate;

	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData MaxUltimate;
	
	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData InitialCheckAmount;
	
	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData InitialApplyAmount;

};

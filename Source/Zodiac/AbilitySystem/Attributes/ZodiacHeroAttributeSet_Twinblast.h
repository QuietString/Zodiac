// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacAttributeSet.h"
#include "ZodiacHeroAttributeSet_Twinblast.generated.h"

UCLASS()
class ZODIAC_API UZodiacHeroAttributeSet_Twinblast : public UZodiacAttributeSet
{
	GENERATED_BODY()

public:
	UZodiacHeroAttributeSet_Twinblast();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	ATTRIBUTE_ACCESSORS(UZodiacHeroAttributeSet_Twinblast, Fuel);
	ATTRIBUTE_ACCESSORS(UZodiacHeroAttributeSet_Twinblast, MaxFuel);
	ATTRIBUTE_ACCESSORS(UZodiacHeroAttributeSet_Twinblast, FuelCharge);
	ATTRIBUTE_ACCESSORS(UZodiacHeroAttributeSet_Twinblast, FuelConsumption);
	ATTRIBUTE_ACCESSORS(UZodiacHeroAttributeSet_Twinblast, InitialCheckAmount);
	ATTRIBUTE_ACCESSORS(UZodiacHeroAttributeSet_Twinblast, InitialApplyAmount);
	
	mutable FZodiacAttributeEvent OnFuelChanged;
	mutable FZodiacAttributeEvent OnMaxFuelChanged;
	mutable FZodiacAttributeEvent OnOutOfFuel;
	
protected:
	UFUNCTION()
	void OnRep_Fuel(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxFuel(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
private:
	UPROPERTY(ReplicatedUsing=OnRep_Fuel, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData Fuel;

	UPROPERTY(ReplicatedUsing=OnRep_MaxFuel, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData MaxFuel;
	
	bool bOutOfFuel;

	// Store the fuel before any changes 
	float MaxFuelBeforeAttributeChange;
	float FuelBeforeAttributeChange;
	
	UPROPERTY(BlueprintReadOnly, Category="Hero|Twinblast", meta=(AllowPrivateAccess = true))
	FGameplayAttributeData FuelCharge;

	UPROPERTY(BlueprintReadOnly, Category="Hero|Twinblast", meta=(AllowPrivateAccess = true))
	FGameplayAttributeData FuelConsumption;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Hero|Twinblast", meta=(AllowPrivateAccess = true))
	FGameplayAttributeData InitialCheckAmount;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Hero|Twinblast", meta=(AllowPrivateAccess = true))
	FGameplayAttributeData InitialApplyAmount;
};

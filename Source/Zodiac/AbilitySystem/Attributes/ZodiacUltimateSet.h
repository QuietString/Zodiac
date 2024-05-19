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

	ATTRIBUTE_ACCESSORS(UZodiacUltimateSet, UltimateGauge);
	ATTRIBUTE_ACCESSORS(UZodiacUltimateSet, MaxUltimateGauge);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UFUNCTION()
	void OnRep_UltimateGauge(const FGameplayAttributeData& OldValue);

private:

	UPROPERTY(ReplicatedUsing=OnRep_UltimateGauge, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData UltimateGauge;

	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData MaxUltimateGauge;
};

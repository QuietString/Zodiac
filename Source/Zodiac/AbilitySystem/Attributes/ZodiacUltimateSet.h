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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_Ultimate(const FGameplayAttributeData& OldValue);

private:

	UPROPERTY(ReplicatedUsing=OnRep_Ultimate, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData Ultimate;

	UPROPERTY(Replicated, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData MaxUltimate;
};

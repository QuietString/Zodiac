// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacAttributeSet.h"
#include "ZodiacCombatSet.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacCombatSet : public UZodiacAttributeSet
{
	GENERATED_BODY()

public:
	UZodiacCombatSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	ATTRIBUTE_ACCESSORS(UZodiacCombatSet, Mana);
protected:
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);

private:
	UPROPERTY(ReplicatedUsing=OnRep_Mana, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData Mana;
};

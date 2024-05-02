// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacAttributeSet.h"
#include "ZodiacCombatSet.generated.h"

/**
 *  Class that defines attributes that are necessary for applying damage or healing.
 *	Attribute examples include: damage, healing, attack power, and shield penetrations.
 */
UCLASS()
class ZODIAC_API UZodiacCombatSet : public UZodiacAttributeSet
{
	GENERATED_BODY()

public:
	
	UZodiacCombatSet();

	ATTRIBUTE_ACCESSORS(UZodiacCombatSet, BaseDamage);
	ATTRIBUTE_ACCESSORS(UZodiacCombatSet, BaseHeal);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	
	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseHeal(const FGameplayAttributeData& OldValue);

private:
	
	UPROPERTY(ReplicatedUsing=OnRep_BaseDamage, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData BaseDamage;

	UPROPERTY(ReplicatedUsing=OnRep_BaseHeal, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FGameplayAttributeData BaseHeal;
};

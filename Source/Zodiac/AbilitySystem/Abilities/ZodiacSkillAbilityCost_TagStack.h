// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacSkillAbilityCost.h"
#include "ZodiacSkillAbilityCost_TagStack.generated.h"

/**
 * Represents a cost that requires expending a quantity of a tag stack
 */
UCLASS(meta=(DisplayName="Tag Stack"))
class ZODIAC_API UZodiacSkillAbilityCost_TagStack : public UZodiacSkillAbilityCost
{
	GENERATED_BODY()

public:
	UZodiacSkillAbilityCost_TagStack();

	//~UZodiacSkillAbilityCost interface
	virtual bool CheckCost(const UZodiacHeroAbility* SkillAbility, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const UZodiacHeroAbility* SkillAbility, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~End of UZodiacSkillAbilityCost interface

	float GetQuantity() const { return Quantity.GetValueAtLevel(0); }
	FGameplayTag GetTag() const { return Tag; }
	
protected:
	/** How much of the tag to spend */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FScalableFloat Quantity;

	/** Which tag to spend some of */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FGameplayTag Tag;

	/** Which tag to send back as a response if this cost cannot be applied */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Costs)
	FGameplayTag FailureTag;
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHeroAbilityDefinition.h"
#include "ZodiacHeroAbilityFragment_CostAttribute.generated.h"

UCLASS()
class ZODIAC_API UZodiacHeroAbilityFragment_CostAttribute : public UZodiacHeroAbilityFragment
{
	GENERATED_BODY()

public:
	virtual void OnSlotCreated(UZodiacHeroAbilitySlot* InSlot) const override;
	
public:
	UPROPERTY(EditAnywhere)
	FGameplayAttribute Attribute;
};

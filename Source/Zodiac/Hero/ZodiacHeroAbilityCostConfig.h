// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHeroAbilityCostConfig.generated.h"

USTRUCT(BlueprintType)
struct ZODIAC_API FZodiacHeroAbilityCostConfig
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Instanced, Category = Costs)
	TArray<TObjectPtr<UZodiacAbilityCost>> AdditionalCosts;
	
	// use different amount of cost for initiation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs")
	bool bHasInitialCost = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs", meta = (EditCondition = "bHasInitialCost"))
	float InitialCostAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs")
	float CostAmount = 0;
};
	
// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHeroAbilityCostConfig.h"
#include "Slot/ZodiacHeroAbilitySlotDefinition.h"
#include "ZodiacHeroAbilityFragment_CostConfig.generated.h"

UCLASS(DisplayName = "Cost Config")
class ZODIAC_API UZodiacHeroAbilityFragment_CostConfig : public UZodiacHeroAbilityFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FZodiacHeroAbilityCostConfig Config;
};

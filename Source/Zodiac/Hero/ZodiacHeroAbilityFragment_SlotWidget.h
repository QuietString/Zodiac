// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHeroAbilityDefinition.h"
#include "ZodiacHeroAbilityFragment_SlotWidget.generated.h"

class UZodiacAbilitySlotWidgetBase;

UCLASS()
class ZODIAC_API UZodiacHeroAbilityFragment_SlotWidget : public UZodiacHeroAbilityFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Reticle)
	TSubclassOf<UZodiacAbilitySlotWidgetBase> Widget;
};

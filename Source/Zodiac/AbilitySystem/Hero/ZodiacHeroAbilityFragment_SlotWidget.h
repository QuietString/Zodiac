﻿// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Slot/ZodiacHeroAbilitySlotDefinition.h"
#include "ZodiacHeroAbilityFragment_SlotWidget.generated.h"

class UZodiacAbilitySlotWidgetBase;

UCLASS(DisplayName = "Slot Widget")
class ZODIAC_API UZodiacHeroAbilityFragment_SlotWidget : public UZodiacHeroAbilityFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UZodiacAbilitySlotWidgetBase> Widget;
};

﻿// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "ZodiacAbilitySlotWidgetBase.generated.h"

class UZodiacHeroAbilitySlot;

UCLASS()
class ZODIAC_API UZodiacAbilitySlotWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UZodiacAbilitySlotWidgetBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable)
	virtual void InitializeFromAbilitySlot(UZodiacHeroAbilitySlot* InSlot);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnHeroChanged();

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnAbilitySlotInitialized();
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Slot")
	TObjectPtr<UZodiacHeroAbilitySlot> AbilitySlot;
};

// the.quiet.string@gmail.com

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

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UZodiacHeroAbilitySlot> AbilitySlot;
};

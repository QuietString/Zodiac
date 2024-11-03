// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ZodiacHeroAbilityWidgetData.generated.h"

class UZodiacAbilitySlotWidgetBase;

USTRUCT(BlueprintType)
struct FZodiacHeroAbilityWidgets
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories="HUD.Type.AbilitySlot"))
	TMap<FGameplayTag, TObjectPtr<UZodiacAbilitySlotWidgetBase>> SlotWidgets;
};

UCLASS(BlueprintType)
class ZODIAC_API UZodiacHeroAbilityWidgetData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FName, FZodiacHeroAbilityWidgets> HeroWidgets;
};

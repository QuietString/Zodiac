// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "ZodiacSkillSlotWidgetBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTagStackChanged, FGameplayTag, Tag, int32, OldCount, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChanged, EZodiacAttributeType, Attribute, float, OldValue, float, NewValue);

UCLASS(Abstract, Blueprintable, BlueprintType)
class UZodiacSkillSlotWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	bool IsMessageForThisWidget(const APawn* Instigator, const int32 InHeroIndex) const;

	UFUNCTION(BlueprintCallable)
	bool IsMessageForThisWidgetWithSlotType(const APawn* Instigator, const int32 InHeroIndex, const FGameplayTag InSlotType) const;
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnTagStackChanged OnTagStackChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChanged OnAttributeChanged;
	
public:
	UPROPERTY(BlueprintReadWrite)
	int32 HeroIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag SlotType;
};

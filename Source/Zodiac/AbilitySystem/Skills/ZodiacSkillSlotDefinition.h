// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ZodiacSkillSlotDefinition.generated.h"

class UZodiacAbilitySet;
class UZodiacSkillSlotWidgetBase;

UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class ZODIAC_API UZodiacSkillSlotFragment : public UObject
{
	GENERATED_BODY()
};

/**
 *  Definition of a piece of a skill that can be used by a hero
 */
UCLASS(BlueprintType, Const)
class ZODIAC_API UZodiacSkillSlotDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UZodiacAbilitySet> SkillSetToGrant;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced)
	TArray<TObjectPtr<UZodiacSkillSlotFragment>> Fragments;
	
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Ability.Stack"))
	TMap<FGameplayTag, int32> InitialTagStack;

	UPROPERTY(EditDefaultsOnly, meta=(DisplayThumbnail="true", DisplayName="Slot Widget"))
	TSubclassOf<UZodiacSkillSlotWidgetBase> SlotWidgetClass;
};

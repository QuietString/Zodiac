// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHeroItemSlot.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "System/GameplayTagStack.h"
#include "UObject/Object.h"
#include "ZodiacSkillInstance.generated.h"

class UZodiacSkillSlotWidgetBase;
class UZodiacAbilitySystemComponent;
class UZodiacSkillInstance;

UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class ZODIAC_API UZodiacSkillSlotFragment : public UObject
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FZodiacSkillSlotDefinition
{
	GENERATED_BODY()

public:
	UZodiacSkillInstance* CreateInstance(AActor* InOwner) const;

public:
	UPROPERTY(EditDefaultsOnly, meta=(Categories="HUD.Type.SkillSlot"))
	FGameplayTag SlotType;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UZodiacAbilitySet> SkillSetToGrant;
	
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Ability.Stack"))
	TMap<FGameplayTag, int32> InitialTagStack;

	UPROPERTY(EditDefaultsOnly, meta=(DisplayThumbnail="true", DisplayName="Slot Widget"))
	TSubclassOf<UZodiacSkillSlotWidgetBase> SlotWidgetClass;
};

/**
 * Instanced skill object.
 */
UCLASS()
class ZODIAC_API UZodiacSkillInstance : public UZodiacHeroItemSlot
{
	GENERATED_BODY()

public:
	UZodiacSkillInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
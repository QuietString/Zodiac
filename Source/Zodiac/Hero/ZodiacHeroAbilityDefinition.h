// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZodiacHeroAbilityDefinition.generated.h"

class UZodiacHeroAbilityFragment;
class UZodiacAbilitySet;
class UZodiacHeroAbilitySlot;

UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class ZODIAC_API UZodiacHeroAbilityFragment : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void OnSlotCreated(UZodiacHeroAbilitySlot* InSlot) const {}
};

USTRUCT(BlueprintType)
struct FZodiacHeroAbilityDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, meta=(Categories="HUD.Type.SkillSlot"))
	FGameplayTag SlotType;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UZodiacHeroAbilitySlot> SlotClass;
	
	UPROPERTY(EditDefaultsOnly, NotReplicated)
	TObjectPtr<UZodiacAbilitySet> SkillSetToGrant;
	
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Ability.Cost.Stack"), NotReplicated)
	TMap<FGameplayTag, int32> InitialTagStack;

	UPROPERTY(EditDefaultsOnly)
	TArray<FName> SourceSockets;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced)
	TArray<UZodiacHeroAbilityFragment*> Fragments;
};
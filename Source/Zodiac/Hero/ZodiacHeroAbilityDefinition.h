// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHeroAbilityDefinition.generated.h"

class UZodiacHeroAbilityFragment;
class UZodiacHeroAbilitySlot;

UCLASS(DefaultToInstanced, EditInlineNew, Abstract, CollapseCategories)
class ZODIAC_API UZodiacHeroAbilityFragment : public UObject
{
	GENERATED_BODY()
	
public:

	UZodiacHeroAbilityFragment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void OnSlotCreated(UZodiacHeroAbilitySlot* InSlot) const {}
};
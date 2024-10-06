// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacSkillInstance.h"
#include "ZodiacSkillSlotFragment_SlotIcon.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacSkillSlotFragment_SlotIcon : public UZodiacSkillSlotFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSlateBrush Brush;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;
};

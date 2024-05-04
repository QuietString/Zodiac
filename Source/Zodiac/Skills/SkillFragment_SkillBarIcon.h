// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacSkillDefinition.h"
#include "SkillFragment_SkillBarIcon.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API USkillFragment_SkillBarIcon : public UZodiacSkillFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush Brush;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FText DisplayNameWhenEquipped;
};


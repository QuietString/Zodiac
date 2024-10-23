// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHeroAbilityDefinition.h"
#include "ZodiacHeroAbilityFragment_Reticle.generated.h"

class UZodiacReticleWidgetBase;

UCLASS(DisplayName = "Reticle Widget")
class ZODIAC_API UZodiacHeroAbilityFragment_Reticle : public UZodiacHeroAbilityFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Reticle)
	TArray<TSubclassOf<UZodiacReticleWidgetBase>> ReticleWidgets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Reticle)
	bool bIsMainReticle = false;
};

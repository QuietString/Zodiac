// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacTraversalTypes.h"
#include "UObject/Interface.h"
#include "ZodiacTraversalChooserInterface.generated.h"

UINTERFACE(BlueprintType)
class UZodiacTraversalChooserInterface : public UInterface
{
	GENERATED_BODY()
};

class ZODIAC_API IZodiacTraversalChooserInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	TArray<UAnimMontage*> EvaluateTraversalChooser(FZodiacTraversalChooserParams ChooserParams);
};

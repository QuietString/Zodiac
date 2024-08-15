// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ZodiacInteractionTransformInterface.generated.h"

UINTERFACE(BlueprintType)
class UZodiacInteractionTransformInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ZODIAC_API IZodiacInteractionTransformInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, meta = (BlueprintThreadSafe))
	void SetInteractionTransform(FTransform InteractionTransform);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, meta = (BlueprintThreadSafe))
	FTransform GetInteractionTransform();
};

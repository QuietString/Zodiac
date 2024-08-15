// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacTraversalTypes.h"
#include "UObject/Interface.h"
#include "ZodiacTraversable.generated.h"

UINTERFACE()
class UZodiacTraversableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ZODIAC_API IZodiacTraversableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void GetLedgeTransforms(FVector HitLocation, FVector ActorLocation, FZodiacTraversalCheckResult& TraversalCheckResultInOut);
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ZodiacUtilityLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacUtilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Global Names")
	static FName GetBloodDecalTag();
};

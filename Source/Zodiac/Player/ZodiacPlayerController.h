// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "ZodiacPlayerController.generated.h"

/**
 * AZodiacPlayerController
 *
 * The base controller class used by this project.
 */
UCLASS(Config = Game, meta = (ShortToolTip = "The base player controller class used by this project."))
class ZODIAC_API AZodiacPlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

	AZodiacPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

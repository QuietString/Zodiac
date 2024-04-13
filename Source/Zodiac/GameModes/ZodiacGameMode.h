// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "ZodiacGameMode.generated.h"

/**
 * AZodiacGameMode
 *
 * The base game mode class used by this project.
 */
UCLASS(Config = Game, meta = (ShortToolTip = "The base game mode class used by this project."))
class ZODIAC_API AZodiacGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	AZodiacGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

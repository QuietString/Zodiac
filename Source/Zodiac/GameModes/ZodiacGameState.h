// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ModularGameState.h"
#include "ZodiacGameState.generated.h"

/**
 * AZodiacGameState
 *
 *	The base game state class used by this project.
 */
UCLASS(Config = Game)
class ZODIAC_API AZodiacGameState : public AModularGameStateBase
{
	GENERATED_BODY()
};

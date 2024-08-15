// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ZodiacGameMode.generated.h"


UCLASS(Config = Game, meta = (ShortToolTip = "The base game mode class used by this project."))
class ZODIAC_API AZodiacGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AZodiacGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
};

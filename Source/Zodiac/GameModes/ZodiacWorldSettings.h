// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "ZodiacWorldSettings.generated.h"

UCLASS()
class ZODIAC_API AZodiacWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:

#if WITH_EDITORONLY_DATA
	// Is this level part of a front-end or other standalone experience?
	// When set, the net mode will be forced to Standalone when you hit Play in the editor
	UPROPERTY(EditDefaultsOnly, Category=PIE)
	bool ForceStandaloneNetMode = false;
#endif
	
};

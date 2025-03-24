// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ZodiacAISubsystem.generated.h"

class AZodiacZombieSpawner;
/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacAISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void RegisterSpawner(AZodiacZombieSpawner* Spawner);
	void UnregisterSpawner(AZodiacZombieSpawner* Spawner);

	void PauseAllMonsters();
	void ResumeAllMonsters();
	
protected:
	// Store spawners in weak pointers so they auto-cleanup if destroyed
	UPROPERTY()
	TArray<TWeakObjectPtr<AZodiacZombieSpawner>> RegisteredSpawners;
};

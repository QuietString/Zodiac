// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZodiacZombieSpawner.generated.h"

class AZodiacMonster;

UCLASS(BlueprintType, Blueprintable)
class ZODIAC_API AZodiacZombieSpawner : public AActor
{
	GENERATED_BODY()

public:
	void SpawnMonsters();

protected:
	virtual void BeginPlay() override;

protected:
	// The zombie character class to spawn
	UPROPERTY(EditAnywhere, Category = "Monster")
	TMap<TSubclassOf<AZodiacMonster>, uint8> MonstersToSpawn;

	// The radius within which monsters will be spawned
	UPROPERTY(EditAnywhere, Category = "Monster")
	float SpawnRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Monster")
	bool bIsEnabled = true;
};

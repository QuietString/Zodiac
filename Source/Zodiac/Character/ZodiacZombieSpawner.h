// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZodiacZombieSpawner.generated.h"

class UEnvQuery;
struct FEnvQueryResult;
class AZodiacMonster;

UCLASS(BlueprintType, Blueprintable)
class ZODIAC_API AZodiacZombieSpawner : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	void SpawnAllMonsters();

protected:
	void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result, TMap<TSubclassOf<AZodiacMonster>, uint8> MonsterToSpawnMap);

	AZodiacMonster* SpawnMonster(const TSubclassOf<AZodiacMonster> ClassToSpawn, const FVector& SpawnLocation);
	
	UFUNCTION()
	void OnMonsterDestroyed(AActor* DestroyedActor);
	
protected:
	UPROPERTY(EditAnywhere, Category = "Spawner")
	bool bIsEnabled = true;
	
	// The zombie character class to spawn
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TMap<TSubclassOf<AZodiacMonster>, uint8> MonstersToSpawn;

	// Half length of spawn area.
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TObjectPtr<UEnvQuery> LocationQuery;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	float GridSize = 500.f;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	float SpawnSpacing = 120.f;
	
private:
	UPROPERTY()
	TSet<TObjectPtr<AZodiacMonster>> SpawnedMonsters;

	int32 TotalMonstersToSpawn;
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZodiacZombieSpawner.generated.h"

class UBehaviorTree;
class UEnvQuery;
struct FEnvQueryResult;
class AZodiacMonster;
struct FZodiacZombieSpawnConfig;

UCLASS(BlueprintType, Blueprintable)
class ZODIAC_API AZodiacZombieSpawner : public AActor
{
	GENERATED_BODY()

protected:
	void RegisterToSubsystem();
	virtual void BeginPlay() override;

public:
	void SpawnAllMonsters();

	TSet<TObjectPtr<AZodiacMonster>> GetAllMonsters() const { return SpawnedMonsters; }
	
	bool GetSpawnOnBeginPlay() const { return bSpawnOnBeginPlay; }
	bool GetUseTrigger() const { return bUseTrigger; }
	
protected:
	void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result, TMap<TSubclassOf<AZodiacMonster>, uint8> MonsterToSpawnMap, FZodiacZombieSpawnConfig SpawnConfig);
	
	AZodiacMonster* SpawnMonster(const TSubclassOf<AZodiacMonster> ClassToSpawn, const FVector& SpawnLocation, FZodiacZombieSpawnConfig ZombieSpawnConfig);

	UFUNCTION()
	void OnMonsterDestroyed(AActor* DestroyedActor);

protected:
	UPROPERTY(EditAnywhere, Category = "Spawner|Spawn")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, Category = "Spawner|Spawn")
	bool bUseTrigger = false;
	
	UPROPERTY(EditAnywhere, Category = "Spawner|Spawn")
	bool bRespawnWhenDies;

	// Half length of spawn area.
	UPROPERTY(EditAnywhere, Category = "Spawner|Spawn")
	TObjectPtr<UEnvQuery> LocationQuery;

	UPROPERTY(EditAnywhere, Category = "Spawner|Spawn")
	bool bSelectCloserLocationToSpawner = true;
	
	// The zombie character class to spawn
	UPROPERTY(EditAnywhere, Category = "Spawner|Pawn")
	TMap<TSubclassOf<AZodiacMonster>, uint8> MonstersToSpawn;

	UPROPERTY(EditAnywhere, Category = "Spawner|Pawn", meta = (UIMin = 0 , UIMax = 1, ClampMin = 0, ClampMax = 1))
	float WalkingRatioWeight = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "Spawner|Pawn", meta = (UIMin = 0 , UIMax = 1, ClampMin = 0, ClampMax = 1))
	float RunningRatioWeight = 0.f;

	UPROPERTY(EditAnywhere, Category = "Spawner|Pawn", meta = (UIMin = 0, UIMax = 1, ClampMin = 0, ClampMax = 1))
	float SprintingRatioWeight = 0.2f;
	
	UPROPERTY(EditAnywhere, Category = "Spawner|Pawn")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, Category = "Spawner|Pawn", meta = (ClampMin = 0))
	float TargetSearchRadius = 10000.f;
	
	UPROPERTY(EditAnywhere, Category = "Spawner|Spacing")
	float GridSize = 500.f;

	UPROPERTY(EditAnywhere, Category = "Spawner|Spacing")
	float SpawnSpacing = 120.f;

	UPROPERTY(EditAnywhere, Category = "Spawner|Spacing", meta = (UIMin = 0, UIMax = 1, ClampMin = 0, ClampMax = 1))
	float RandomSkipping = 0.f;

private:
	UPROPERTY()
	TSet<TObjectPtr<AZodiacMonster>> SpawnedMonsters;
	
	int32 TotalNumberToInitialSpawn;
	
	// Tracks how many monsters of each class are waiting to respawn.
	UPROPERTY()
	TMap<TSubclassOf<AZodiacMonster>, uint8> AccumulatedRespawnRequests;

	// How many monsters to respawn at once, per type.
	UPROPERTY(EditAnywhere, Category = "Spawner|Spawn", meta = (ClampMin = "1"))
	int32 BunchRespawnSize = 4;
};

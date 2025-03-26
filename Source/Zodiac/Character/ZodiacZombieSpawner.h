// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacCharacterType.h"
#include "GameFramework/Actor.h"
#include "ZodiacZombieSpawner.generated.h"

class UBehaviorTree;
class UEnvQuery;
struct FEnvQueryResult;
class AZodiacMonster;

USTRUCT()
struct FPendingSpawnInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TSubclassOf<AZodiacMonster> MonsterClass;

	UPROPERTY()
	FVector SpawnLocation;

	// Any other data you need, e.g. random seeds, MovementMode, etc.
	// We'll keep it simple here.
	FZodiacZombieSpawnConfig SpawnConfig;

	FPendingSpawnInfo()
		: MonsterClass(nullptr), SpawnLocation(FVector::ZeroVector) {}

	FPendingSpawnInfo(TSubclassOf<AZodiacMonster> InClass, const FVector& InLoc, const FZodiacZombieSpawnConfig& InConfig)
		: MonsterClass(InClass), SpawnLocation(InLoc), SpawnConfig(InConfig) {}
};

UCLASS(BlueprintType, Blueprintable)
class ZODIAC_API AZodiacZombieSpawner : public AActor
{
	GENERATED_BODY()

public:
	AZodiacZombieSpawner(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
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
	void StartDeferredSpawning();

	virtual void Tick(float DeltaTime) override;

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

	UPROPERTY(EditAnywhere, Category = "Spawner|Spawn")
	TObjectPtr<UEnvQuery> LocationQuery;

	UPROPERTY(EditAnywhere, Category = "Spawner|Spawn")
	bool bSelectCloserLocationToSpawner = true;

	// How many monsters we spawn per frame in the “batched” approach
	UPROPERTY(EditAnywhere, Category="Spawner|Spawn")
	int32 SpawnsPerFrame = 5;
	
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

	// If false, no distance limit for finding target actor.
	UPROPERTY(EditAnywhere, Category = "Spawner|Pawn")
	bool bUseTargetSearchRadius = false;
	
	UPROPERTY(EditAnywhere, Category = "Spawner|Pawn", meta = (ClampMin = 0, EditCondition = "bUseTargetSearchRadius"))
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

	// Keep an array (or queue) of “pending” spawns we will process
	TArray<FPendingSpawnInfo> PendingSpawns;

	// Whether we’re currently spawning in a deferred manner
	bool bDeferredSpawningInProgress = false;
	
	// Tracks how many monsters of each class are waiting to respawn.
	UPROPERTY()
	TMap<TSubclassOf<AZodiacMonster>, uint8> AccumulatedRespawnRequests;

	// How many monsters to respawn at once, per type.
	UPROPERTY(EditAnywhere, Category = "Spawner|Spawn", meta = (ClampMin = "1"))
	int32 RespawnBatchSize = 4;
};

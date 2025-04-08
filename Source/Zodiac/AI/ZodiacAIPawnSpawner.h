// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Character/ZodiacCharacterType.h"
#include "GameFramework/Actor.h"
#include "ZodiacAIPawnSpawner.generated.h"

class UZodiacAIPawnSubsystem;
class UBehaviorTree;
class UEnvQuery;
struct FEnvQueryResult;
class AZodiacMonster;

UCLASS(BlueprintType, Blueprintable)
class ZODIAC_API AZodiacAIPawnSpawner : public AActor
{
	GENERATED_BODY()

public:
	AZodiacAIPawnSpawner(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
protected:
	void RegisterToSubsystem();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

public:
	void SpawnAllMonstersFromPool();
	
	FZodiacZombieSpawnConfig GenerateSpawnConfig();

	void AddMonstersToPool();
	
	void SendAllMonstersBackToPool();
	void SendMonsterBackToPool(AZodiacMonster* MonsterToSend);
	
	UFUNCTION()
	void OnPawnDeathFinished(AActor* DeadActor);
	
	UFUNCTION()
	void OnPawnReadyToRespawn(AActor* SleepingActor);

	void TryBatchSpawn();

	UFUNCTION()
	void PerformQueuedSpawn(const TMap<TSubclassOf<AZodiacMonster>, uint8>& MonsterToSpawnMap);
	
protected:
	void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result, TMap<TSubclassOf<AZodiacMonster>, uint8> MonsterToSpawnMap);

	AZodiacMonster* SpawnMonsterFromPool(const TSubclassOf<AZodiacMonster>& ClassToSpawn, const FVector& SpawnLocation);

	friend UZodiacAIPawnSubsystem;
	
public:
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

	UPROPERTY(EditAnywhere, Category = "Spawner|Spawn", meta = (ClampMin = 0))
	int32 MinimumPartialSpawnCount = 0;
	
	UPROPERTY(editAnywhere, Category = "Spawner|Spawn")
	float RespawnDelay = 1.f;
	
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
	int32 TotalNumberOfInitialSpawn;
	
	// Tracks how many monsters of each class are waiting to respawn.
	UPROPERTY(Transient)
	TMap<TSubclassOf<AZodiacMonster>, uint8> ReadyToRespawnMap;

	// Queued respawn requests in AIPawnSubsystem.
	UPROPERTY(Transient)
	TMap<TSubclassOf<AZodiacMonster>, uint8> PendingRespawnRequests;
	
	// How many monsters to respawn at once. We use batch to execute EQS less frequently.
	UPROPERTY(EditAnywhere, Category = "Spawner|Spawn", meta = (ClampMin = "1"))
	int32 RespawnBatchSize = 10;

	UPROPERTY(Transient)
	TObjectPtr<UZodiacAIPawnSubsystem> AIPawnSubsystem;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AZodiacMonster>> SpawnedMonsters;
};

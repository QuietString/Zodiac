// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Character/ZodiacMonster.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ZodiacAIPawnSpawner.h"
#include "ZodiacAIPawnSubsystem.generated.h"

enum class EZodiacExtendedMovementMode : uint8;
struct FZodiacZombieSpawnConfig;
class AZodiacMonster;

USTRUCT()
struct FSpawnRequest
{
	GENERATED_BODY()

	// Which spawner wants to spawn
	UPROPERTY()
	TWeakObjectPtr<AZodiacAIPawnSpawner> Spawner;

	// The monster classes and how many of each we want
	UPROPERTY()
	TMap<TSubclassOf<AZodiacMonster>, uint8> MonsterToSpawnMap;
    
	FSpawnRequest() {}
    
	FSpawnRequest(const AZodiacAIPawnSpawner* InSpawner, const TMap<TSubclassOf<AZodiacMonster>, uint8>& InMonsterMap)
		: Spawner(const_cast<AZodiacAIPawnSpawner*>(InSpawner))
		, MonsterToSpawnMap(InMonsterMap)
	{}
};

USTRUCT()
struct FZodiacAIPawnClassPool
{
	GENERATED_BODY()

	// Which class this array is for
	UPROPERTY()
	TSubclassOf<AZodiacMonster> MonsterClass;

	UPROPERTY()
	TArray<TObjectPtr<AZodiacMonster>> Instances;
};

USTRUCT()
struct FZodiacSpawnerPool
{
	GENERATED_BODY()

	// Which spawner this pool belongs to
	UPROPERTY()
	TWeakObjectPtr<AZodiacAIPawnSpawner> Spawner;

	// List of each monster class and its array of monsters
	UPROPERTY()
	TArray<FZodiacAIPawnClassPool> ClassPools;
};

/**
 * A subsystem for managing AI pawns
 */
UCLASS(Config=Spawner)
class ZODIAC_API UZodiacAIPawnSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void RegisterSpawner(AZodiacAIPawnSpawner* Spawner);
	void UnregisterSpawner(AZodiacAIPawnSpawner* Spawner);

	FZodiacSpawnerPool* FindSpawnerPool(AZodiacAIPawnSpawner* Spawner);
	FZodiacSpawnerPool& FindOrAddSpawnerPool(AZodiacAIPawnSpawner* Spawner);
	FZodiacAIPawnClassPool& FindOrAddMonsterClassPool(FZodiacSpawnerPool& SpawnerPool, const TSubclassOf<AZodiacMonster>& MonsterClass);
	
	void AddMonsterToPool(AZodiacAIPawnSpawner* Spawner, const TSubclassOf<AZodiacMonster>& ClassToSpawn, const FZodiacZombieSpawnConfig& SpawnConfig);
	void SendMonsterBackToPool(AZodiacAIPawnSpawner* Spawner, AZodiacMonster* Monster);

	void HatchAllPawnsFromPool();
	void SendAllPawnsBackToPool();
	
	AZodiacMonster* HatchMonsterFromPool(AZodiacAIPawnSpawner* Spawner, const TSubclassOf<AZodiacMonster>& RequestedClass);

	void QueueSpawnRequest(AZodiacAIPawnSpawner* Spawner, const TMap<TSubclassOf<AZodiacMonster>, uint8>& RequestedMap);
	void ProcessSpawnRequests();
	void SplitSpawnRequest(const TMap<TSubclassOf<AZodiacMonster>, uint8>& OriginalRequest,	int32 Capacity,	TMap<TSubclassOf<AZodiacMonster>, uint8>& OutPartialRequest, TMap<TSubclassOf<AZodiacMonster>, uint8>& OutLeftoverRequest);

	UFUNCTION(BlueprintCallable)
	int32 GetNumberOfActivePawns() const;

	UFUNCTION(BlueprintPure)
	int32 GetCurrentSpawnLimit() const;

	void NotifySpawnFinished(AZodiacAIPawnSpawner* Spawner);
	
	// Cheats
	void PauseAllMonsters();
	void ResumeAllMonsters();

	void SpawnDebugPawns();
	void KillDebugPawns();

protected:
	void DespawnByTimeOut(float DeltaTime);

	bool Tick(float DeltaTime);

#if WITH_EDITORONLY_DATA | !UE_BUILD_SHIPPING
	UFUNCTION(BlueprintCallable)
	void PrintCurrentState() const;
#endif
	
#if !UE_BUILD_SHIPPING
public:
	static void PrintSpawnStateConsoleCommand(UWorld* World);
protected:
	bool DebugTick(float DeltaTime);
	void MakeDebugMessage(FString& Msg) const;
	void DisplayDebugMessages();
#endif
	
public:
	// How long (in seconds) a monster can be target-less before being despawned
	UPROPERTY(BlueprintReadWrite, Config, Category="AI", meta=(ClampMin="0.0"))
	float TargetActorLostTimeout;

	// How many pawns exists concurrently
	UPROPERTY(BlueprintReadWrite, Config, Category="AI", meta=(ClampMin="0"))
	int32 MaxGlobalAIPawns;
	
protected:
	UPROPERTY()
	TArray<FZodiacSpawnerPool> SpawnerPools;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<AZodiacAIPawnSpawner>> RegisteredSpawners;
	
	UPROPERTY(Transient)
	TSet<TObjectPtr<AZodiacMonster>> ActiveMonsters;
	
	// A queue of pending spawn requests that couldn’t spawn immediately
	UPROPERTY()
	TArray<FSpawnRequest> SpawnRequestsQueue;
	
	// How long each monster has been target-less
	UPROPERTY(Transient)
	TMap<TObjectPtr<AZodiacMonster>, float> TimeWithoutTarget;
	
private:
	UPROPERTY(Config)
	float TickInterval;
	
	// Since ZodiacAIPawnSpawner uses EQS query which is asynchronous, we have to cache spawn count until it actually spawn them to calculate capacity.
	UPROPERTY(Transient)
	TMap<TObjectPtr<AZodiacAIPawnSpawner>, int32> CachedNumberOfSpawning;

	FTSTicker::FDelegateHandle TickHandle;

#if !UE_BUILD_SHIPPING
	FTSTicker::FDelegateHandle DebugTickHandle;
#endif
};
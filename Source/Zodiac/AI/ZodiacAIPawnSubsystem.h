// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Character/ZodiacMonster.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ZodiacAIPawnSubsystem.generated.h"

class AZodiacAIPawnSpawner;
enum class EZodiacExtendedMovementMode : uint8;
struct FZodiacZombieSpawnConfig;
class AZodiacMonster;
class AZodiacZombieSpawner;

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
	TWeakObjectPtr<const AZodiacAIPawnSpawner> Spawner;

	// List of each monster class and its array of monsters
	UPROPERTY()
	TArray<FZodiacAIPawnClassPool> ClassPools;
};

/**
 * A subsystem for managing AI pawns
 */
UCLASS()
class ZODIAC_API UZodiacAIPawnSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void RegisterSpawner(AZodiacAIPawnSpawner* Spawner);
	void UnregisterSpawner(AZodiacAIPawnSpawner* Spawner);

	FZodiacSpawnerPool* FindSpawnerPool(const AZodiacAIPawnSpawner* Spawner);
	FZodiacSpawnerPool& FindOrAddSpawnerPool(const AZodiacAIPawnSpawner* Spawner);
	FZodiacAIPawnClassPool& FindOrAddMonsterClassPool(FZodiacSpawnerPool& SpawnerPool, const TSubclassOf<AZodiacMonster>& MonsterClass);
	
	void AddMonsterToPool(const AZodiacAIPawnSpawner* Spawner, const TSubclassOf<AZodiacMonster>& ClassToSpawn, const FZodiacZombieSpawnConfig& SpawnConfig);
	void ReleaseMonsterToPool(const AZodiacAIPawnSpawner* Spawner, AZodiacMonster* Monster);

	AZodiacMonster* HatchMonsterFromPool(const AZodiacAIPawnSpawner* Spawner, const TSubclassOf<AZodiacMonster>& RequestedClass);
	
	void PauseAllMonsters();
	void ResumeAllMonsters();
	
protected:
	UPROPERTY()
	TArray<FZodiacSpawnerPool> SpawnerPools;
	
	// Store spawners in weak pointers so they auto-cleanup if destroyed
	UPROPERTY()
	TArray<TWeakObjectPtr<AZodiacAIPawnSpawner>> RegisteredSpawners;
};

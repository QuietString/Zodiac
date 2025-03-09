// the.quiet.string@gmail.com

#include "ZodiacZombieSpawner.h"

#include "ZodiacLogChannels.h"
#include "ZodiacMonster.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "System/ZodiacGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacZombieSpawner)

namespace ZombieSpawnerQueryParamNames
{
	FName GridSize = FName("SimpleGrid.GridSize");
	FName SpaceBetween = FName("SimpleGrid.SpaceBetween");
}

void AZodiacZombieSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bIsEnabled)
	{
		SpawnAllMonsters();
	}
}

void AZodiacZombieSpawner::SpawnAllMonsters()
{
	if (!HasAuthority() || !LocationQuery)
	{
		return;
	}
	
	if (LocationQuery)
	{
		FEnvQueryRequest Request(LocationQuery, this);
		Request.SetFloatParam(ZombieSpawnerQueryParamNames::GridSize, GridSize);
		Request.SetFloatParam(ZombieSpawnerQueryParamNames::SpaceBetween, SpawnSpacing);
		
		FQueryFinishedSignature QueryFinishedDelegate = FQueryFinishedSignature::CreateLambda(
	[this](TSharedPtr<FEnvQueryResult> Res)
			{
				OnQueryFinished(Res, MonstersToSpawn, FZodiacZombieSpawnConfig());
			}
		);
		
		Request.Execute(EEnvQueryRunMode::AllMatching, QueryFinishedDelegate);
	}
}

void AZodiacZombieSpawner::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result, TMap<TSubclassOf<AZodiacMonster>, uint8> MonsterToSpawnMap, FZodiacZombieSpawnConfig SpawnConfig)
{
	if (!Result.IsValid() || !Result->IsSuccessful())
	{
		UE_LOG(LogZodiacSpawner, Log, TEXT("Initial EQS query failed or no results."));
		return;
	}

	TArray<FVector> SpawnPoints;
	Result->GetAllAsLocations(SpawnPoints);

	int32 TotalNumber = 0;
	for (auto& Pair : MonsterToSpawnMap)
	{
		TotalNumber += Pair.Value;
	}
	
	if (SpawnPoints.Num() < TotalNumber)
	{
		UE_LOG(LogZodiacSpawner, Log, TEXT("Not enough space to spawn monsters. Number to spawn: %d, Available locations: %d."), TotalNumber, SpawnPoints.Num());
		return;
	}

	int32 PointIndex = 0;

	for (auto& Pair : MonsterToSpawnMap)
	{
		TSubclassOf<AZodiacMonster> MonsterClass = Pair.Key;
		int32 NumberToSpawn = Pair.Value;

		int32 SpawnCount = 0;
		while (SpawnCount < NumberToSpawn && PointIndex < SpawnPoints.Num())
		{
			FVector Location = SpawnPoints[PointIndex++];
			AActor* NewMonster = SpawnMonster2(MonsterClass, Location, SpawnConfig);
			if (NewMonster) SpawnCount++;
		}
	}
}

AZodiacMonster* AZodiacZombieSpawner::SpawnMonster2(const TSubclassOf<AZodiacMonster> ClassToSpawn, const FVector& SpawnLocation, FZodiacZombieSpawnConfig ZombieSpawnConfig)
{
	UWorld* World = GetWorld();
	if (!World || !ClassToSpawn)
	{
		return nullptr;
	}

	const UZodiacGameData& GameData = UZodiacGameData::Get();
	check(&GameData);

	FRotator SpawnRotation(0.f, FMath::RandRange(-180.f, 180.f), 0.f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	AZodiacMonster* Spawned = World->SpawnActor<AZodiacMonster>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
	if (Spawned)
	{
		if (ZombieSpawnConfig.IsValid())
		{
			Spawned->SetSpawnConfig(ZombieSpawnConfig);
		}
		else
		{
			TArray<FZodiacExtendedMovementConfig> MovementConfigs = GameData.MovementConfigTemplates;
			if (!MovementConfigs.IsEmpty())
			{
				int32 ConfigsNum = MovementConfigs.Num();
				int32 RandomIndex = FMath::RandRange(0, ConfigsNum -1);
				FZodiacExtendedMovementConfig PickedConfig = MovementConfigs[RandomIndex];

				float r = FMath::FRand();
				EZodiacExtendedMovementMode Mode = (r <= RunningRatio) ? EZodiacExtendedMovementMode::Running : EZodiacExtendedMovementMode::Walking;
		
				int32 RandomSeed = FMath::RandRange(0, UINT8_MAX);
				FZodiacZombieSpawnConfig SpawnConfig = FZodiacZombieSpawnConfig(RandomIndex, Mode, RandomSeed);
		
				Spawned->SetSpawnConfig(SpawnConfig);
			}	
		}
		
		Spawned->SpawnDefaultController();
		
		SpawnedMonsters.Add(Spawned);
		
		// Bind OnDestroyed so we can do a re-spawn
		Spawned->OnDestroyed.AddDynamic(this, &AZodiacZombieSpawner::OnMonsterDestroyed);
	}
	
	return Spawned;
}

void AZodiacZombieSpawner::OnMonsterDestroyed(AActor* DestroyedActor)
{
	AZodiacMonster* DeadMonster = Cast<AZodiacMonster>(DestroyedActor);
	if (!DeadMonster)
	{
		return;
	}

	SpawnedMonsters.Remove(DeadMonster);

	// We want to re-spawn the same class
	TSubclassOf<AZodiacMonster> MonsterClass = DeadMonster->GetClass(); 
	TMap<TSubclassOf<AZodiacMonster>, uint8> SpawnNumberMap;
	SpawnNumberMap.Add(DeadMonster->GetClass(), 1);

	if (LocationQuery)
	{
		FEnvQueryRequest Request(LocationQuery, this);
		Request.SetFloatParam(ZombieSpawnerQueryParamNames::GridSize, GridSize);
		Request.SetFloatParam(ZombieSpawnerQueryParamNames::SpaceBetween, SpawnSpacing);

		FZodiacZombieSpawnConfig SpawnConfig = DeadMonster->GetZombieSpawnConfig();
		
		FQueryFinishedSignature QueryFinishedDelegate = FQueryFinishedSignature::CreateLambda(
	[this, SpawnNumberMap, SpawnConfig](TSharedPtr<FEnvQueryResult> Res)
			{
				OnQueryFinished(Res, SpawnNumberMap, SpawnConfig);
			}
		);
		
		// Execute the query in SingleResult mode
		Request.Execute(EEnvQueryRunMode::SingleResult, QueryFinishedDelegate);
	}
}

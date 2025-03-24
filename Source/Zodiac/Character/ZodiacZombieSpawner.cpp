// the.quiet.string@gmail.com

#include "ZodiacZombieSpawner.h"

#include "AIController.h"
#include "ZodiacLogChannels.h"
#include "ZodiacMonster.h"
#include "AI/ZodiacAISubsystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "System/ZodiacGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacZombieSpawner)

namespace ZombieSpawnerQueryParamNames
{
	FName GridSize = FName("SimpleGrid.GridSize");
	FName SpaceBetween = FName("SimpleGrid.SpaceBetween");
}

void AZodiacZombieSpawner::RegisterToSubsystem()
{
	// Register this spawner to ZodiacAISubsystem.
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UZodiacAISubsystem* AISubsystem = GI->GetSubsystem<UZodiacAISubsystem>())
			{
				AISubsystem->RegisterSpawner(this);
			}
		}
	}
}

void AZodiacZombieSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() || !bIsEnabled)
	{
		return;
	}

	RegisterToSubsystem();

	TotalNumberToInitialSpawn = 0;
	for (auto& [K, V] : MonstersToSpawn)
	{
		TotalNumberToInitialSpawn += V;
	}

	if (bSpawnOnBeginPlay)
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
	
	// 1. Gather all item locations + scores
	TArray<FVector> AllLocations;
	Result->GetAllAsLocations(AllLocations);

	// 2. Count how many monsters total
	int32 TotalNumber = 0;
	for (auto& Pair : MonsterToSpawnMap)
	{
		TotalNumber += Pair.Value;
	}

	if (AllLocations.Num() < TotalNumber)
	{
		UE_LOG(LogZodiacSpawner, Warning, TEXT("%s: Not enough space to spawn monsters. Need %d, but have %d points."),
			*GetName(), TotalNumber, AllLocations.Num());
		return;
	}
	
	// We'll make a small struct to store location + score
	struct FLocationScore
	{
		FVector Location;
		float Score;
	};

	TArray<FLocationScore> LocationScores;
	LocationScores.Reserve(AllLocations.Num());

	for (int32 i = 0; i < AllLocations.Num(); i++)
	{
		FLocationScore LS;
		LS.Location = AllLocations[i];
		LS.Score = Result->GetItemScore(i);
		LocationScores.Add(LS);
	}

	if (bSelectCloserLocationToSpawner)
	{
		// 3. Sort by score (descending)
		LocationScores.Sort([](const FLocationScore& A, const FLocationScore& B)
		{
			return A.Score > B.Score; // highest score first
		});	
	}
	
	// We'll iterate through MonsterToSpawnMap, spawning in the top-scored locations first
	int32 PointIndex = 0;

	for (auto& Pair : MonsterToSpawnMap)
	{
		TSubclassOf<AZodiacMonster> MonsterClass = Pair.Key;
		int32 NumberToSpawn = Pair.Value;

		int32 SpawnCount = 0;
		while (SpawnCount < NumberToSpawn && PointIndex < LocationScores.Num())
		{
			FVector SpawnLocation = LocationScores[PointIndex].Location;
			PointIndex++;

			if (FMath::FRand() < RandomSkipping)
			{
				// Skip for random spacing
				continue;
			}

			if (AActor* NewMonster = SpawnMonster(MonsterClass, SpawnLocation, SpawnConfig))
			{
				SpawnCount++;
			}
		}

		if (SpawnCount < NumberToSpawn)
		{
			UE_LOG(LogZodiacSpawner, Warning, TEXT("%s couldn't spawn %d %ss due to a shortage of spawn locations"), *GetName(), NumberToSpawn - SpawnCount, *MonsterClass->GetName());
		}
	}
}

AZodiacMonster* AZodiacZombieSpawner::SpawnMonster(const TSubclassOf<AZodiacMonster> ClassToSpawn, const FVector& SpawnLocation, FZodiacZombieSpawnConfig ZombieSpawnConfig)
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
		Spawned->bIsSpawnedBySpawner = true;
		
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

				float WeightSum = WalkingRatioWeight + RunningRatioWeight + SprintingRatioWeight;
				if (WeightSum <= 0.f)
				{
					WeightSum = 1.f;
				}
				float WalkingRatio = WalkingRatioWeight / WeightSum;
				float RunningRatio = RunningRatioWeight / WeightSum;
				float SprintingRatio = SprintingRatioWeight / WeightSum;
				
				float r = FMath::FRand();
				EZodiacExtendedMovementMode Mode;
				
				if (r < WalkingRatio)
				{
					Mode = EZodiacExtendedMovementMode::Walking;
				}
				else if (r >= WalkingRatio && r < WalkingRatio + RunningRatio)
				{
					Mode = EZodiacExtendedMovementMode::Running;
				}
				else
				{
					Mode = EZodiacExtendedMovementMode::Sprinting;
				}
				
				int32 RandomSeed = FMath::RandRange(0, UINT8_MAX);
				FZodiacZombieSpawnConfig SpawnConfig = FZodiacZombieSpawnConfig(RandomIndex, Mode, RandomSeed);
				
				Spawned->SetSpawnConfig(SpawnConfig);
			}	
		}
		
		Spawned->SpawnDefaultController();
		if (AAIController* AC = Spawned->GetController<AAIController>())
		{
			AC->RunBehaviorTree(BehaviorTree);
			if (UBlackboardComponent* BlackboardComponent = AC->GetBlackboardComponent())
			{
				BlackboardComponent->SetValueAsFloat(FName("SearchRadius"), TargetSearchRadius);
			}
		}
		
		SpawnedMonsters.Add(Spawned);

		if (bRespawnWhenDies)
		{
			// Bind OnDestroyed so we can do a re-spawn
			Spawned->OnDestroyed.AddDynamic(this, &AZodiacZombieSpawner::OnMonsterDestroyed);	
		}
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

	if (!bRespawnWhenDies)
	{
		return;
	}

	TSubclassOf<AZodiacMonster> MonsterClass = DeadMonster->GetClass();
	const int32 NewCount = AccumulatedRespawnRequests.FindOrAdd(MonsterClass) + 1;
	AccumulatedRespawnRequests[MonsterClass] = NewCount;

	int32 TotalRequestCounts = 0;
	for (auto& [K, V] : AccumulatedRespawnRequests)
	{
		TotalRequestCounts += V;
	}
	
	// Check if we reached our batch threshold
	if ((TotalRequestCounts >= BunchRespawnSize) || BunchRespawnSize >= TotalNumberToInitialSpawn)
	{
		if (LocationQuery)
		{
			TMap<TSubclassOf<AZodiacMonster>, uint8> SpawnNumberMap = AccumulatedRespawnRequests;

			FEnvQueryRequest Request(LocationQuery, this);
			Request.SetFloatParam(ZombieSpawnerQueryParamNames::GridSize, GridSize);
			Request.SetFloatParam(ZombieSpawnerQueryParamNames::SpaceBetween, SpawnSpacing);

			// We'll pass in a small callback lambda capturing the local TMap
			FZodiacZombieSpawnConfig SpawnConfig = DeadMonster->GetZombieSpawnConfig();
			FQueryFinishedSignature QueryFinishedDelegate = FQueryFinishedSignature::CreateLambda(
				[this, SpawnNumberMap, SpawnConfig](TSharedPtr<FEnvQueryResult> Res)
				{
					OnQueryFinished(Res, SpawnNumberMap, SpawnConfig);
				}
			);
            
			Request.Execute(EEnvQueryRunMode::RandomBest25Pct, QueryFinishedDelegate);

			AccumulatedRespawnRequests.Empty();
		}
	}
}
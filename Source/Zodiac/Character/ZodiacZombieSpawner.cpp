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

AZodiacZombieSpawner::AZodiacZombieSpawner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
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

	if (!HasAuthority())
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

    // Gather all item locations + their scores
    TArray<FVector> AllLocations;
    Result->GetAllAsLocations(AllLocations);

    // If you have to get item scores, do it now
    TArray<float> AllScores;
    AllScores.Reserve(AllLocations.Num());
    for (int32 i = 0; i < AllLocations.Num(); ++i)
    {
        AllScores.Add(Result->GetItemScore(i));
    }

    // We copy the data so the lambda can operate on it in a background thread
    // (We can't safely reference 'Result' or big arrays that might go out of scope.)
    TMap<TSubclassOf<AZodiacMonster>, uint8> LocalMonsterMap = MonsterToSpawnMap;
    bool bCloserLocationToSpawner = bSelectCloserLocationToSpawner;
    float LocalRandomSkipping = RandomSkipping;

    // 1) Launch background task to compute final spawn transforms:
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=, this]()
    {
        // This array will hold the final result: a list of FPendingSpawnInfo
        TArray<FPendingSpawnInfo> ComputedSpawns;
        ComputedSpawns.Reserve(5000); // Just guess a max. Optional optimization

        // Sort or process your AllLocations/AllScores as needed:
        // Example: if bCloserLocationToSpawner => sort descending by score
        TArray<int32> SortedIndices;
        SortedIndices.Reserve(AllLocations.Num());
        for (int32 i = 0; i < AllLocations.Num(); i++)
        {
            SortedIndices.Add(i);
        }

        if (bCloserLocationToSpawner)
        {
            // Sort indices by descending score
            SortedIndices.Sort([&AllScores](int32 A, int32 B){
                return AllScores[A] > AllScores[B];
            });
        }

        // Now create spawn info for each monster in LocalMonsterMap
        int32 IndexForLocations = 0;
        for (auto& Pair : LocalMonsterMap)
        {
            TSubclassOf<AZodiacMonster> MonsterClass = Pair.Key;
            int32 NumberNeeded = Pair.Value;

            int32 SpawnCount = 0;
            while (SpawnCount < NumberNeeded && IndexForLocations < SortedIndices.Num())
            {
                const int32 LocIdx = SortedIndices[IndexForLocations++];
                // Possibly skip for random skipping
                if (FMath::FRand() < LocalRandomSkipping)
                {
                    continue;
                }

                FVector ChosenLocation = AllLocations[LocIdx];

                // Build and add a new spawn info struct
                FPendingSpawnInfo Info(MonsterClass, ChosenLocation, SpawnConfig);
                ComputedSpawns.Add(Info);
                SpawnCount++;
            }
        }

        // 2) Once we have the TArray<FPendingSpawnInfo>, we need to pass it back
        // to the game thread. We'll use a lambda posted to the game thread:
        AsyncTask(ENamedThreads::GameThread, [this, ComputedSpawns]()
        {
            // Now we’re back on the game thread:

            // Append to our spawner’s PendingSpawns
            PendingSpawns.Append(ComputedSpawns);

            if (!bDeferredSpawningInProgress)
            {
                bDeferredSpawningInProgress = true;
                // Start spawning them in small batches each tick
                StartDeferredSpawning();
            }
        });
    });
}

void AZodiacZombieSpawner::StartDeferredSpawning()
{
	SetActorTickEnabled(true);
}

void AZodiacZombieSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Spawn up to SpawnsPerFrame monsters
	int32 NumToSpawnThisFrame = FMath::Min(SpawnsPerFrame, PendingSpawns.Num());
	for (int32 i = 0; i < NumToSpawnThisFrame; i++)
	{
		const FPendingSpawnInfo& Info = PendingSpawns[i];
		SpawnMonster(Info.MonsterClass, Info.SpawnLocation, Info.SpawnConfig);
	}
    
	// Remove them from the front
	PendingSpawns.RemoveAt(0, NumToSpawnThisFrame);
	
	// If no more pending spawns, disable ticking
	if (PendingSpawns.Num() == 0)
	{
		bDeferredSpawningInProgress = false;
		SetActorTickEnabled(false);
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
	if ((TotalRequestCounts >= RespawnBatchSize) || RespawnBatchSize >= TotalNumberToInitialSpawn)
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
// the.quiet.string@gmail.com

#include "ZodiacAIPawnSpawner.h"


#include "ZodiacLogChannels.h"
#include "AI/ZodiacMonster.h"
#include "AI/ZodiacAIPawnSubsystem.h"
#include "Algo/RandomShuffle.h"
#include "Character/ZodiacHealthComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "System/ZodiacGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAIPawnSpawner)

namespace ZombieSpawnerQueryParamNames
{
	FName GridSize = FName("SimpleGrid.GridSize");
	FName SpaceBetween = FName("SimpleGrid.SpaceBetween");
}

AZodiacAIPawnSpawner::AZodiacAIPawnSpawner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetReplicates(false);
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AZodiacAIPawnSpawner::RegisterToSubsystem()
{
	// Register this spawner to ZodiacAISubsystem.
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UZodiacAIPawnSubsystem* AISubsystem = GI->GetSubsystem<UZodiacAIPawnSubsystem>())
			{
				AISubsystem->RegisterSpawner(this);
				AIPawnSubsystem = AISubsystem;
			}
		}
	}
}

void AZodiacAIPawnSpawner::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (!HasAuthority())
	{
		return;
	}

	RegisterToSubsystem();
}

void AZodiacAIPawnSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority())
	{
		return;
	}

	check(AIPawnSubsystem);

	TotalNumberOfInitialSpawn = 0;
	for (auto& [K, V] : MonstersToSpawn)
	{
		TotalNumberOfInitialSpawn += V;
	}

	AddMonstersToPool();
	
	if (bSpawnOnBeginPlay)
	{
		SpawnAllMonstersFromPool();
	}
}

void AZodiacAIPawnSpawner::SpawnAllMonstersFromPool()
{
	if (!HasAuthority())
	{
		return;
	}

	PendingRespawnRequests = MonstersToSpawn;
	AIPawnSubsystem->QueueSpawnRequest(this, PendingRespawnRequests);
}

FZodiacZombieSpawnConfig AZodiacAIPawnSpawner::GenerateSpawnConfig()
{
	UWorld* World = GetWorld();
	check(World);

	const UZodiacGameData& GameData = UZodiacGameData::Get();
	check(&GameData);
	
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
		
		int32 RandomSeed = FMath::RandRange(1, UINT8_MAX);
		
		return FZodiacZombieSpawnConfig(RandomIndex, Mode, RandomSeed, BehaviorTree, bUseTargetSearchRadius, TargetSearchRadius, WaitTimeAfterSpawn, WaitTimeRandomDeviation, bRespawnWhenDies, bAllowSwitchingExtendedMovementMode);
	}

	return FZodiacZombieSpawnConfig();
}

void AZodiacAIPawnSpawner::AddMonstersToPool()
{
	UWorld* World = GetWorld();
	check(World);

	UGameInstance* GameInstance = GetGameInstance();
	check(GameInstance);

	check(AIPawnSubsystem);
	
	for (auto& [K, V] : MonstersToSpawn)
	{
		for (int i = 0; i < V; i++)
		{
			FZodiacZombieSpawnConfig SpawnConfig = GenerateSpawnConfig();
			AIPawnSubsystem->AddMonsterToPool(this, K, SpawnConfig);	
		}
	}
}

void AZodiacAIPawnSpawner::SendAllMonstersBackToPool()
{
	TArray<AZodiacMonster*> LocalCopy = SpawnedMonsters;
	
	for (auto& SpawnedMonster : LocalCopy)
	{
		SendMonsterBackToPool(SpawnedMonster);
	}
}

void AZodiacAIPawnSpawner::SendMonsterBackToPool(AZodiacMonster* MonsterToSend)
{
	SpawnedMonsters.Remove(MonsterToSend);
	
	MonsterToSend->Multicast_Sleep();
	
	if (AIPawnSubsystem)
	{
		AIPawnSubsystem->SendMonsterBackToPool(this, MonsterToSend);
	}
}

void AZodiacAIPawnSpawner::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result, TMap<TSubclassOf<AZodiacMonster>, uint8> MonsterToSpawnMap)
{
	if (!Result.IsValid() || !Result->IsSuccessful())
    {
        UE_LOG(LogZodiacSpawner, Log, TEXT("%s: EQS failed or no results."), *GetName());
        return;
    }
    if (!AIPawnSubsystem)
    {
        UE_LOG(LogZodiacSpawner, Warning, TEXT("No AIPawnSubsystem!"));
        return;
    }

    // 1) Count total spawns
    int32 TotalSpawnCount = 0;
    for (auto& Pair : MonsterToSpawnMap)
    {
        TotalSpawnCount += Pair.Value;
    }

    // 2) Build a single, random-shuffled list of monsters to spawn
    TArray<TSubclassOf<AZodiacMonster>> MonsterSpawnList;
    MonsterSpawnList.Reserve(TotalSpawnCount);
    for (auto& Pair : MonsterToSpawnMap)
    {
        for (int32 i = 0; i < Pair.Value; i++)
        {
            MonsterSpawnList.Add(Pair.Key);
        }
    }
	
    Algo::RandomShuffle(MonsterSpawnList); // Randomize the spawn order

    // 3) Get all EQS locations + scores
    TArray<FVector> AllLocations;
    Result->GetAllAsLocations(AllLocations);

    TArray<float> AllScores;
    AllScores.Reserve(AllLocations.Num());
    for (int32 i = 0; i < AllLocations.Num(); i++)
    {
        AllScores.Add(Result->GetItemScore(i));
    }

    // 4) Sort location indices by descending score if you want closer/higher scored first
    TArray<int32> SortedIndices;
    SortedIndices.Reserve(AllLocations.Num());
    for (int32 i = 0; i < AllLocations.Num(); i++)
    {
        SortedIndices.Add(i);
    }
    if (bSelectCloserLocationToSpawner)
    {
        SortedIndices.Sort([&AllScores](int32 A, int32 B){
            return AllScores[A] > AllScores[B];
        });
    }

    // 5) Assign each monster from the shuffled list to the next best location
    int32 LocIndex = 0;
    for (int32 i = 0; i < MonsterSpawnList.Num(); i++)
    {
        if (LocIndex >= SortedIndices.Num())
        {
            // no more locations
            break;
        }

        // Possibly skip for random spacing
        if (FMath::FRand() < RandomSkipping)
        {
            continue;
        }

        TSubclassOf<AZodiacMonster> ThisClass = MonsterSpawnList[i];
        int32 LocIdx = SortedIndices[LocIndex++];
        FVector SpawnLoc = AllLocations[LocIdx];

        if (AZodiacMonster* Spawned = SpawnMonsterFromPool(ThisClass, SpawnLoc))
        {
            SpawnedMonsters.AddUnique(Spawned);
        }
    }

    AIPawnSubsystem->NotifySpawnFinished(this);
}

AZodiacMonster* AZodiacAIPawnSpawner::SpawnMonsterFromPool(const TSubclassOf<AZodiacMonster>& ClassToSpawn, const FVector& SpawnLocation)
{
	UWorld* World = GetWorld();
	if (!World || !ClassToSpawn)
	{
		return nullptr;
	}

	if (!AIPawnSubsystem)
	{
		return nullptr;
	}

	FRotator SpawnRotation(0.f, FMath::RandRange(-180.f, 180.f), 0.f);
	
	if (AZodiacMonster* Fetched = AIPawnSubsystem->HatchMonsterFromPool(this, ClassToSpawn))
	{
		if (UZodiacHealthComponent* HealthComponent = UZodiacHealthComponent::FindHealthComponent(Fetched))
		{
			HealthComponent->OnDeathFinished.AddUniqueDynamic(this, &ThisClass::OnPawnDeathFinished);
		}
		
		Fetched->Multicast_WakeUp(SpawnLocation, SpawnRotation);

		return Fetched;
	}
	
	return nullptr;
}

void AZodiacAIPawnSpawner::OnPawnDeathFinished(AActor* DeadActor)
{
	AZodiacMonster* DeadMonster = Cast<AZodiacMonster>(DeadActor);
	if (!DeadMonster)
	{
		return;
	}

	SendMonsterBackToPool(DeadMonster);
	
	if (!bRespawnWhenDies)
	{
		return;
	}

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this, DeadActor]()
	{
		OnPawnReadyToRespawn(DeadActor);
	},
	RespawnDelay, false);
}

void AZodiacAIPawnSpawner::OnPawnReadyToRespawn(AActor* SleepingActor)
{
	AZodiacMonster* SleepingMonster = Cast<AZodiacMonster>(SleepingActor);
	if (!SleepingMonster || !AIPawnSubsystem)
	{
		return;
	}
	
	TSubclassOf<AZodiacMonster> MonsterClass = SleepingMonster->GetClass();
	const int32 NewCount = ReadyToRespawnMap.FindOrAdd(MonsterClass) + 1;
	ReadyToRespawnMap[MonsterClass] = NewCount;

	TryBatchSpawn();
}

void AZodiacAIPawnSpawner::TryBatchSpawn()
{
	int32 TotalRequestCounts = 0;
	for (auto& [K, V] : ReadyToRespawnMap)
	{
		TotalRequestCounts += V;
	}

	// Respawn when the number of accumulated requests reaches RespawnBatchSize,
	// or the number of initial spawn was less than RespawnBatchSize.
	bool bShouldBatchRespawn = (TotalRequestCounts >= RespawnBatchSize) ? true : (RespawnBatchSize >= TotalNumberOfInitialSpawn);
	if (bShouldBatchRespawn && PendingRespawnRequests.IsEmpty())
	{
		PendingRespawnRequests = ReadyToRespawnMap;
		AIPawnSubsystem->QueueSpawnRequest(this, PendingRespawnRequests);
		ReadyToRespawnMap.Empty();
	}
}

void AZodiacAIPawnSpawner::PerformQueuedSpawn(const TMap<TSubclassOf<AZodiacMonster>, uint8>& MonsterToSpawnMap)
{
	if (!HasAuthority() || !LocationQuery)
	{
		return;
	}

	// Make a local copy to update
	TMap<TSubclassOf<AZodiacMonster>, uint8> RespawnLeftOvers = PendingRespawnRequests;
	PendingRespawnRequests.Empty();

	for (const TPair<TSubclassOf<AZodiacMonster>, uint8>& Pair : MonsterToSpawnMap)
	{
		const TSubclassOf<AZodiacMonster>& MonsterClass = Pair.Key;
		const uint8 SpawnRequest = Pair.Value;

		// If we have leftover requests for this class
		if (RespawnLeftOvers.Contains(MonsterClass))
		{
			uint8 OldCount = RespawnLeftOvers[MonsterClass];
            
			if (SpawnRequest >= OldCount)
			{
				RespawnLeftOvers.Remove(MonsterClass);
			}
			else
			{
				RespawnLeftOvers[MonsterClass] = OldCount - SpawnRequest;
			}
		}
	}

	for (auto& [K, V] : RespawnLeftOvers)
	{
		const int32 NewCount = ReadyToRespawnMap.FindOrAdd(K) + 1;
		ReadyToRespawnMap[K] = NewCount;
	}
	
	FEnvQueryRequest Request(LocationQuery, this);
	Request.SetFloatParam(ZombieSpawnerQueryParamNames::GridSize, GridSize);
	Request.SetFloatParam(ZombieSpawnerQueryParamNames::SpaceBetween, SpawnSpacing);

	FQueryFinishedSignature QueryFinishedDelegate = FQueryFinishedSignature::CreateLambda(
		[this, MonsterToSpawnMap](TSharedPtr<FEnvQueryResult> Res)
		{
			OnQueryFinished(Res, MonsterToSpawnMap);
		}
	);
	Request.Execute(EEnvQueryRunMode::AllMatching, QueryFinishedDelegate);
}
// the.quiet.string@gmail.com

#include "ZodiacAIPawnSpawner.h"


#include "ZodiacLogChannels.h"
#include "Character/ZodiacMonster.h"
#include "AI/ZodiacAIPawnSubsystem.h"
#include "Character/ZodiacHealthComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "System/ZodiacGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAIPawnSpawner)

namespace ZombieSpawnerQueryParamNames2
{
	FName GridSize = FName("SimpleGrid.GridSize");
	FName SpaceBetween = FName("SimpleGrid.SpaceBetween");
}

AZodiacAIPawnSpawner::AZodiacAIPawnSpawner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetReplicates(false);
	PrimaryActorTick.bCanEverTick = true;
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
	if (!HasAuthority() || !LocationQuery)
	{
		return;
	}
	
	if (LocationQuery)
	{
		FEnvQueryRequest Request(LocationQuery, this);
		Request.SetFloatParam(ZombieSpawnerQueryParamNames2::GridSize, GridSize);
		Request.SetFloatParam(ZombieSpawnerQueryParamNames2::SpaceBetween, SpawnSpacing);
		
		FQueryFinishedSignature QueryFinishedDelegate = FQueryFinishedSignature::CreateLambda(
	[this](TSharedPtr<FEnvQueryResult> Res)
			{
				OnQueryFinished(Res, MonstersToSpawn);
			}
		);
		
		Request.Execute(EEnvQueryRunMode::AllMatching, QueryFinishedDelegate);
	}
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
		
		int32 RandomSeed = FMath::RandRange(0, UINT8_MAX);
		
		return FZodiacZombieSpawnConfig(RandomIndex, Mode, RandomSeed, BehaviorTree, bUseTargetSearchRadius, TargetSearchRadius, bRespawnWhenDies);
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

void AZodiacAIPawnSpawner::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result, TMap<TSubclassOf<AZodiacMonster>, uint8> MonsterToSpawnMap)
{
	if (!Result.IsValid() || !Result->IsSuccessful())
	{
		UE_LOG(LogZodiacSpawner, Log, TEXT("%s: Initial EQS query failed or no results."), *GetName());
		return;
	}

	// Gather all item locations + their scores
	TArray<FVector> AllLocations;
	Result->GetAllAsLocations(AllLocations);
	
	TArray<float> AllScores;
	AllScores.Reserve(AllLocations.Num());
	for (int32 i = 0; i < AllLocations.Num(); ++i)
	{
		AllScores.Add(Result->GetItemScore(i));
	}

	TArray<int32> SortedIndices;
	SortedIndices.Reserve(AllLocations.Num());
	for (int32 i = 0; i < AllLocations.Num(); i++)
	{
		SortedIndices.Add(i);
	}

	if (bSelectCloserLocationToSpawner)
	{
		// Sort indices by descending score
		SortedIndices.Sort([&AllScores](int32 A, int32 B){
			return AllScores[A] > AllScores[B];
		});
	}
	
	int32 IndexForLocations = 0;
	for (auto& Pair : MonsterToSpawnMap)
	{
		TSubclassOf<AZodiacMonster> MonsterClass = Pair.Key;
		int32 NumberNeeded = Pair.Value;

		int32 SpawnCount = 0;
		while (SpawnCount < NumberNeeded && IndexForLocations < SortedIndices.Num())
		{
			const int32 LocIdx = SortedIndices[IndexForLocations++];
			// Possibly skip for random skipping
			if (FMath::FRand() < RandomSkipping)
			{
				continue;
			}

			FVector ChosenLocation = AllLocations[LocIdx];

			AZodiacMonster* Spawned = SpawnMonsterFromPool(MonsterClass, ChosenLocation);
			SpawnCount++;

			if (Spawned)
			{
				SpawnedMonsters.AddUnique(Spawned);
				if (UZodiacHealthComponent* HealthComponent = UZodiacHealthComponent::FindHealthComponent(Spawned))
				{
					HealthComponent->OnDeathFinished.AddUniqueDynamic(this, &ThisClass::OnPawnDeathFinished);
				}            			
			}
			else
			{
				UE_LOG(LogZodiacSpawner, Warning, TEXT("Failed to spawn %s from the pool"), *MonsterClass->GetName());
			}
		}
	}
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

	SpawnedMonsters.Remove(DeadMonster);
	
	DeadMonster->Multicast_Sleep();
	
	if (AIPawnSubsystem)
	{
		AIPawnSubsystem->ReleaseMonsterToPool(this, DeadMonster);
	}

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

void AZodiacAIPawnSpawner::TryBatchSpawn()
{
	int32 TotalRequestCounts = 0;
	for (auto& [K, V] : AccumulatedRespawnRequests)
	{
		TotalRequestCounts += V;
	}

	// Respawn when the number of accumulated requests reaches RespawnBatchSize,
	// or the number of initial spawn was less than RespawnBatchSize.
	bool bShouldBatchRespawn = (TotalRequestCounts >= RespawnBatchSize) ? true : (RespawnBatchSize >= TotalNumberOfInitialSpawn);
	if (bShouldBatchRespawn)
	{
		if (LocationQuery)
		{
			TMap<TSubclassOf<AZodiacMonster>, uint8> SpawnNumberMap = AccumulatedRespawnRequests;

			FEnvQueryRequest Request(LocationQuery, this);
			Request.SetFloatParam(ZombieSpawnerQueryParamNames2::GridSize, GridSize);
			Request.SetFloatParam(ZombieSpawnerQueryParamNames2::SpaceBetween, SpawnSpacing);

			// We'll pass in a small callback lambda capturing the local TMap
			FQueryFinishedSignature QueryFinishedDelegate = FQueryFinishedSignature::CreateLambda(
				[this, SpawnNumberMap](TSharedPtr<FEnvQueryResult> Res)
				{
					OnQueryFinished(Res, SpawnNumberMap);
				}
			);
            
			Request.Execute(EEnvQueryRunMode::AllMatching, QueryFinishedDelegate);

			AccumulatedRespawnRequests.Empty();
		}
	}
}

void AZodiacAIPawnSpawner::OnPawnReadyToRespawn(AActor* SleepingActor)
{
	AZodiacMonster* SleepingMonster = Cast<AZodiacMonster>(SleepingActor);
	if (!SleepingMonster)
	{
		return;
	}
	
	TSubclassOf<AZodiacMonster> MonsterClass = SleepingMonster->GetClass();
	const int32 NewCount = AccumulatedRespawnRequests.FindOrAdd(MonsterClass) + 1;
	AccumulatedRespawnRequests[MonsterClass] = NewCount;

	TryBatchSpawn();
}


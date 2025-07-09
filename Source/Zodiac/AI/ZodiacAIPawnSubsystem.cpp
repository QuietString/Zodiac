// the.quiet.string@gmail.com


#include "ZodiacAIPawnSubsystem.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "AI/ZodiacMonster.h"
#include "ZodiacAIPawnSpawner.h"
#include "ZodiacLogChannels.h"
#include "Algo/RandomShuffle.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAIPawnSubsystem)

namespace
{
	/** When true we draw the spawn state every frame */
	bool GShowSpawnState = false;
}

static TAutoConsoleVariable<int32> CVarMaxGlobalAIPawns(
	TEXT("Zodiac.ai.MaxGlobalAIPawns"),
	60,
	TEXT("Maximum global AI pawns at once."),
	ECVF_Default
);

static TAutoConsoleVariable<float> CVar_TargetActorLostTimeout(
	TEXT("Zodiac.ai.TargetActorLostTimeout"),
	60.0f,
	TEXT("How long (in seconds) a monster can be target-less before being despawned."),
	ECVF_Default
);

#if !UE_BUILD_SHIPPING
static FAutoConsoleCommand GPrintSpawnStateCmd(
	TEXT("Zodiac.ai.PrintSpawnState"),
	TEXT("Print current overall spawn state, e.g. Active, Queued, Spawning counts."),
	FConsoleCommandWithWorldDelegate::CreateStatic(
		&UZodiacAIPawnSubsystem::PrintSpawnStateConsoleCommand
	)
);
#endif

static FAutoConsoleCommand GToggleDisplaySpawnStateCmd(
	TEXT("Zodiac.ai.ToggleDisplaySpawnState"),
	TEXT("Toggle continuous on‑screen display of the AI‑spawn state."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		GShowSpawnState = !GShowSpawnState;
		UE_LOG(LogZodiacSpawner, Display, TEXT("Display AI Pawn State: %s"), GShowSpawnState ? TEXT("On") : TEXT("Off"));
	})
);

void UZodiacAIPawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// Set the console variable's default to match the config-laded value:
	IConsoleVariable* TimeOutVar = IConsoleManager::Get().FindConsoleVariable(TEXT("Zodiac.ai.TargetActorLostTimeout"));
	if (TimeOutVar)
	{
		TimeOutVar->Set(TargetActorLostTimeout, ECVF_SetByConstructor);
	}

	IConsoleVariable* MaxPawnVar = IConsoleManager::Get().FindConsoleVariable(TEXT("Zodiac.ai.MaxGlobalAIPawns"));
	if (MaxPawnVar)
	{
		MaxPawnVar->Set(MaxGlobalAIPawns, ECVF_SetByConstructor);
	}
	
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::Tick), TickInterval);

#if !UE_BUILD_SHIPPING
	DebugTickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ThisClass::DebugTick), 0.f);
#endif

}

void UZodiacAIPawnSubsystem::RegisterSpawner(AZodiacAIPawnSpawner* Spawner)
{
	if (!Spawner)
	{
		return;
	}

	RegisteredSpawners.AddUnique(Spawner);
}

void UZodiacAIPawnSubsystem::UnregisterSpawner(AZodiacAIPawnSpawner* Spawner)
{
	if (!Spawner)
	{
		return;
	}
	
	RegisteredSpawners.RemoveAll([Spawner](const TWeakObjectPtr<AZodiacAIPawnSpawner>& WeakPtr)
	{
		return WeakPtr.Get() == Spawner;
	});
}

FZodiacSpawnerPool* UZodiacAIPawnSubsystem::FindSpawnerPool(AZodiacAIPawnSpawner* Spawner)
{
	for (FZodiacSpawnerPool& Entry : SpawnerPools)
	{
		if (Entry.Spawner.Get() == Spawner)
		{
			return &Entry;
		}
	}
	
	return nullptr;
}

FZodiacSpawnerPool& UZodiacAIPawnSubsystem::FindOrAddSpawnerPool(AZodiacAIPawnSpawner* Spawner)
{
	for (FZodiacSpawnerPool& Entry : SpawnerPools)
	{
		if (Entry.Spawner.Get() == Spawner)
		{
			return Entry;
		}
	}

	FZodiacSpawnerPool NewPool;
	NewPool.Spawner = Spawner;

	int32 Index = SpawnerPools.Add(NewPool);
	return SpawnerPools[Index];
}

FZodiacAIPawnClassPool& UZodiacAIPawnSubsystem::FindOrAddMonsterClassPool(FZodiacSpawnerPool& SpawnerPool, const TSubclassOf<AZodiacMonster>& MonsterClass)
{
	for (FZodiacAIPawnClassPool& ACP : SpawnerPool.ClassPools)
	{
		if (ACP.MonsterClass == MonsterClass)
		{
			return ACP;
		}
	}

	FZodiacAIPawnClassPool NewACP;
	NewACP.MonsterClass = MonsterClass;

	int32 NewIndex = SpawnerPool.ClassPools.Add(NewACP);
	return SpawnerPool.ClassPools[NewIndex];
}

void UZodiacAIPawnSubsystem::AddMonsterToPool(AZodiacAIPawnSpawner* Spawner, const TSubclassOf<AZodiacMonster>& ClassToSpawn, const FZodiacZombieSpawnConfig& SpawnConfig)
{
	UWorld* World = GetWorld();
	if (!World || !ClassToSpawn)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = Spawner;
	
	AZodiacMonster* Spawned = World->SpawnActor<AZodiacMonster>(ClassToSpawn, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (!Spawned)
	{
		return;
	}
	
	Spawned->SpawnDefaultController();
	Spawned->Multicast_Sleep();
	
	Spawned->SetSpawnConfig(SpawnConfig);
	Spawned->Spawner = Spawner;
	
	// 1) Find or create the pool for that spawner
	FZodiacSpawnerPool& SpawnerPool = FindOrAddSpawnerPool(Spawner);

	// 2) Find or create the sub-pool for that monster class
	auto& [MonsterClass, Instances] = FindOrAddMonsterClassPool(SpawnerPool, ClassToSpawn);

	// 3) Actually store the monster
	Instances.Add(Spawned);
}

void UZodiacAIPawnSubsystem::SendMonsterBackToPool(AZodiacAIPawnSpawner* Spawner, AZodiacMonster* Monster)
{
	if (!Monster || !Spawner)
	{
		return;
	}

	// 1) Determine the monster’s class
	TSubclassOf<AZodiacMonster> MonsterClass = Monster->GetClass();

	// 2) Find (or create) the spawner’s pool
	FZodiacSpawnerPool& SpawnerPool = FindOrAddSpawnerPool(Spawner);

	// 3) Find (or create) the array of monsters for that class
	FZodiacAIPawnClassPool& ClassPool = FindOrAddMonsterClassPool(SpawnerPool, MonsterClass);

	// 4) Actually store the monster in the pool
	ClassPool.Instances.AddUnique(Monster);
	
	ActiveMonsters.Remove(Monster);

	// See if we can spawn something from the queue
	ProcessSpawnRequests();
}

void UZodiacAIPawnSubsystem::HatchAllPawnsFromPool()
{
	for (auto& SpawnerPool : SpawnerPools)
	{
		if (AZodiacAIPawnSpawner* Spawner = SpawnerPool.Spawner.Get())
		{
			Spawner->SpawnAllMonstersFromPool();
		}
	}
}

void UZodiacAIPawnSubsystem::SendAllPawnsBackToPool()
{
	
	for (auto& SpawnerPool : SpawnerPools)
	{
		if (AZodiacAIPawnSpawner* Spawner = SpawnerPool.Spawner.Get())
		{
			Spawner->SendAllMonstersBackToPool();
		}
	}
}

AZodiacMonster* UZodiacAIPawnSubsystem::HatchMonsterFromPool(AZodiacAIPawnSpawner* Spawner, const TSubclassOf<AZodiacMonster>& RequestedClass)
{
	if (!Spawner || !RequestedClass)
	{
		return nullptr;
	}

	// 1) Find the existing spawner pool
	FZodiacSpawnerPool* SpawnerPoolPtr = FindSpawnerPool(Spawner);
	if (!SpawnerPoolPtr)
	{
		return nullptr;
	}

	// 2) Find the monster class pool
	FZodiacAIPawnClassPool* ClassPoolPtr = nullptr;
	for (FZodiacAIPawnClassPool& ClassPool : SpawnerPoolPtr->ClassPools)
	{
		if (ClassPool.MonsterClass == RequestedClass)
		{
			ClassPoolPtr = &ClassPool;
			break;
		}
	}
	if (!ClassPoolPtr || ClassPoolPtr->Instances.Num() == 0)
	{
		return nullptr;
	}
	
	// 3) Pop from the array
	AZodiacMonster* Monster = ClassPoolPtr->Instances.Pop(EAllowShrinking::No);
	if (Monster)
	{
		ActiveMonsters.Add(Monster);
	}

	return Monster;
}

void UZodiacAIPawnSubsystem::QueueSpawnRequest(AZodiacAIPawnSpawner* Spawner, const TMap<TSubclassOf<AZodiacMonster>, uint8>& RequestedMap)
{
	if (!Spawner)
	{
		return;
	}

	FSpawnRequest NewRequest(Spawner, RequestedMap);
	SpawnRequestsQueue.Add(NewRequest);
	ProcessSpawnRequests();

	UE_LOG(LogZodiacSpawner, Log, TEXT("Spawn request added from %s."), *Spawner->GetActorNameOrLabel());
}

void UZodiacAIPawnSubsystem::ProcessSpawnRequests()
{
	// We'll keep popping from the front of the array (like a queue)
	// while there's enough capacity. 
	
	int32 i = 0;
	while (i < SpawnRequestsQueue.Num())
	{
		const FSpawnRequest& Request = SpawnRequestsQueue[i];
		AZodiacAIPawnSpawner* Spawner = Request.Spawner.Get();
		if (!Spawner)
		{
			SpawnRequestsQueue.RemoveAt(i);
			continue;
		}

		int32 CurrentLimit = CVarMaxGlobalAIPawns.GetValueOnGameThread();
		int32 CurrentlyActive = GetNumberOfActivePawns();
		int32 CurrentlySpawning = 0;
		for (auto& [K, V] : CachedNumberOfSpawning)
		{
			CurrentlySpawning += V;
		}
		
		int32 CapacityLeft = CurrentLimit - CurrentlyActive - CurrentlySpawning;
		if (CapacityLeft <= 0)
        {
            // No capacity left at all; skip spawning further requests
            break;
        }
		
		int32 TotalNeeded = 0;
		for (auto& Pair : Request.MonsterToSpawnMap)
		{
			TotalNeeded += Pair.Value;
		}
		
		if (TotalNeeded <= CapacityLeft)
		{
			// We have enough capacity for the spawn request. 
			Spawner->PerformQueuedSpawn(Request.MonsterToSpawnMap);
			CachedNumberOfSpawning.Add(Spawner, TotalNeeded);
			
			// Remove the request from the queue
			SpawnRequestsQueue.RemoveAt(i);
			
			UE_LOG(LogZodiacSpawner, Log, TEXT("Perform queued spawn request. Requested: %d, Active + Spawning: %d, Capacity left: %d"), TotalNeeded, CurrentlyActive + CurrentlySpawning, CapacityLeft);
		}
		else if (CapacityLeft >= Spawner->MinimumPartialSpawnCount)
		{
			// We only have partial capacity
			TMap<TSubclassOf<AZodiacMonster>, uint8> PartialRequest;
			TMap<TSubclassOf<AZodiacMonster>, uint8> LeftoverRequest;

			// Split the request so we spawn as many as we can
			SplitSpawnRequest(Request.MonsterToSpawnMap, CapacityLeft, PartialRequest, LeftoverRequest);

			// How many are we actually spawning in partial?
			int32 PartialCount = 0;
			for (auto& Pair : PartialRequest)
			{
				PartialCount += Pair.Value;
			}

			if (PartialCount > 0)
			{
				// Perform partial spawn
				Spawner->PerformQueuedSpawn(PartialRequest);
				CachedNumberOfSpawning.Add(Spawner, PartialCount);

				UE_LOG(LogZodiacSpawner, Log, TEXT("Perform partial spawn: %d out of %d. Leftover re-queued."), PartialCount, TotalNeeded);
			}
			else
			{
				// If partialCount is 0, it means capacityLeft was 0 or negative,
				// so we can’t spawn anything.
				// We'll break out of the loop so we wait for capacity next time
				break;
			}

			// The leftover we re-queue at the BACK of SpawnRequestsQueue
			if (LeftoverRequest.Num() > 0)
			{
				FSpawnRequest LeftoverF(Request.Spawner.Get(), LeftoverRequest);
				// push it at the end
				SpawnRequestsQueue.Add(LeftoverF);
			}

			// We remove the original request from the queue
			SpawnRequestsQueue.RemoveAt(i);
			
			break;
		}
		else
		{
			i++;
			continue; // move on to check next request
		}
	}
}

void UZodiacAIPawnSubsystem::SplitSpawnRequest(const TMap<TSubclassOf<AZodiacMonster>, uint8>& OriginalRequest, int32 Capacity, TMap<TSubclassOf<AZodiacMonster>, uint8>& OutPartialRequest,
	TMap<TSubclassOf<AZodiacMonster>, uint8>& OutLeftoverRequest)
{
	OutPartialRequest.Empty();
    OutLeftoverRequest = OriginalRequest;

    if (Capacity <= 0 || OriginalRequest.Num() == 0)
    {
        return;
    }

    // 1. Copy the request into a working array and shuffle its order (shuffling is optional but prevents bias)
    TArray<TPair<TSubclassOf<AZodiacMonster>, uint8>> WorkArray;
    WorkArray.Reserve(OriginalRequest.Num());

    for (const TPair<TSubclassOf<AZodiacMonster>, uint8>& Pair : OriginalRequest)
    {
        WorkArray.Add(Pair);
    }

	// randomise start class
    Algo::RandomShuffle(WorkArray);

    // 2. Round‑robin allocation
    int32 RemainingCapacity = Capacity;
    bool bAnyClassStillNeeds = true;

    while (RemainingCapacity > 0 && bAnyClassStillNeeds)
    {
        bAnyClassStillNeeds = false;

        for (TPair<TSubclassOf<AZodiacMonster>, uint8>& Pair : WorkArray)
        {
            if (RemainingCapacity == 0)
            {
                break;
            }

            if (Pair.Value > 0) // this class still needs pawns
            {
                // Give ONE pawn of this class
                OutPartialRequest.FindOrAdd(Pair.Key)++;
                Pair.Value--;
                RemainingCapacity--;

                bAnyClassStillNeeds = true;
            }
        }
    }

    // 3. Build the leftover map
    OutLeftoverRequest.Empty();
    for (const TPair<TSubclassOf<AZodiacMonster>, uint8>& Pair : WorkArray)
    {
        if (Pair.Value > 0)
        {
            OutLeftoverRequest.Add(Pair.Key, Pair.Value);
        }
    }
}

int32 UZodiacAIPawnSubsystem::GetNumberOfActivePawns() const
{
	return ActiveMonsters.Num();
}

int32 UZodiacAIPawnSubsystem::GetCurrentSpawnLimit() const
{
	return CVarMaxGlobalAIPawns.GetValueOnGameThread();
}

void UZodiacAIPawnSubsystem::NotifySpawnFinished(AZodiacAIPawnSpawner* Spawner)
{
	CachedNumberOfSpawning.Remove(Spawner);
}

void UZodiacAIPawnSubsystem::PauseAllMonsters()
{
	for (auto& Spawner : RegisteredSpawners)
	{
		if (Spawner.IsValid())
		{
			for (auto& Monster : Spawner->SpawnedMonsters)
			{
				if (Monster)
				{
					if (AAIController* AC = Cast<AAIController>(Monster->GetController()))
					{
						FString Reason = TEXT("Paused by ZodiacAIPawnSubsystem");
						AC->BrainComponent->PauseLogic(Reason);
					}
				}
			}
		}
	}
}

void UZodiacAIPawnSubsystem::ResumeAllMonsters()
{
	for (auto& Spawner : RegisteredSpawners)
	{
		if (Spawner.IsValid())
		{
			for (auto& Monster : Spawner->SpawnedMonsters)
			{
				if (Monster)
				{
					if (AAIController* AC = Cast<AAIController>(Monster->GetController()))
					{
						FString Reason = TEXT("Resumed by ZodiacAIPawnSubsystem");
						AC->BrainComponent->ResumeLogic(Reason);
					}
				}
			}
		}
	}
}

void UZodiacAIPawnSubsystem::SpawnDebugPawns()
{
	for (auto& Spawner : RegisteredSpawners)
	{
		if (Spawner.IsValid())
		{
			if (Spawner->ActorHasTag(FName("Debug")))
			{
				// Spawn only when it's not spawned yet.
				if (Spawner->SpawnedMonsters.Num() != Spawner->TotalNumberOfInitialSpawn)
				{
					Spawner->SpawnAllMonstersFromPool();	
				}
				else
				{
					UE_LOG(LogZodiacSpawner, Display, TEXT("Debug pawns already spawned"));
				}
			}
		}
	}
}

void UZodiacAIPawnSubsystem::KillDebugPawns()
{
	for (auto& Spawner : RegisteredSpawners)
	{
		if (Spawner.IsValid())
		{
			if (Spawner->ActorHasTag(FName("Debug")))
			{
				Spawner->SendAllMonstersBackToPool();
			}
		}
	}
}

void UZodiacAIPawnSubsystem::DespawnByTimeOut(float DeltaTime)
{
	TArray<AZodiacMonster*> MonstersToDespawn;
	
	for (AZodiacMonster* Monster : ActiveMonsters)
	{
		if (!Monster)
		{
			continue;	
		}
		
		AZodiacAIPawnSpawner* Spawner = Monster->Spawner.Get();
		
		if (!Spawner || !Spawner->bDespawnWhenNoTarget)
		{
			continue;
		}
		
		// If monster->TargetActor is valid => reset timer
		if (Monster->TargetActor)
		{
			TimeWithoutTarget.FindOrAdd(Monster) = 0.0f;
		}
		else
		{
			// Accumulate time
			float& AccTime = TimeWithoutTarget.FindOrAdd(Monster);
			AccTime += DeltaTime;

			// If we exceed the timeout, mark for despawn
			if (AccTime >= TargetActorLostTimeout)
			{
				MonstersToDespawn.Add(Monster);
			}
		}
	}

	// Now actually despawn those that timed out
	for (AZodiacMonster* Monster : MonstersToDespawn)
	{
		if (Monster && Monster->IsValidLowLevelFast())
		{
			if (AZodiacAIPawnSpawner* Spawner = Monster->Spawner.Get())
			{
				Spawner->SendMonsterBackToPool(Monster);

				TimeWithoutTarget.Remove(Monster);

				UE_LOG(LogZodiacSpawner, Log, TEXT("Monster %s lost target for too long, returning to pool."), *Monster->GetName());
			}
		}
	}
}

bool UZodiacAIPawnSubsystem::Tick(float DeltaTime)
{
	UWorld* World = GetWorld();

	if (!World || World->GetNetMode() > NM_Client)
	{
		return true;
	}
	
	TargetActorLostTimeout = CVar_TargetActorLostTimeout.GetValueOnGameThread();
	
	// Watch out DeltaTime is time difference from previous frame, not difference from previous Tick time.
	DespawnByTimeOut(TickInterval);
	ProcessSpawnRequests();
	
	return true;
}

#if WITH_EDITORONLY_DATA | !UE_BUILD_SHIPPING
void UZodiacAIPawnSubsystem::PrintCurrentState() const
{
	FString DebugMsg;
	MakeDebugMessage(DebugMsg);
	
	UE_LOG(LogZodiacSpawner, Display, TEXT("%s"), *DebugMsg);
}

#endif

#if !UE_BUILD_SHIPPING
bool UZodiacAIPawnSubsystem::DebugTick(float DeltaTime)
{
	if (GShowSpawnState)
	{
		DisplayDebugMessages();
	}
	return true;
}

void UZodiacAIPawnSubsystem::PrintSpawnStateConsoleCommand(UWorld* World)
{
	if (!World) return;
    
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return;

	if (UZodiacAIPawnSubsystem* AISubsystem = GI->GetSubsystem<UZodiacAIPawnSubsystem>())
	{
		AISubsystem->PrintCurrentState();
	}
}

void UZodiacAIPawnSubsystem::MakeDebugMessage(FString& Msg) const
{
	// --- gather numbers ------------------------------------------------
	const int32 CurrentActive = GetNumberOfActivePawns();

	int32 CurrentlySpawning = 0;
	for (const TPair<TObjectPtr<AZodiacAIPawnSpawner>, int32>& KV : CachedNumberOfSpawning)
	{
		CurrentlySpawning += KV.Value;
	}

	int32 QueuedReq  = SpawnRequestsQueue.Num();
	int32 QueuedPawns = 0;
	for (const FSpawnRequest& Req : SpawnRequestsQueue)
	{
		for (const auto& Pair : Req.MonsterToSpawnMap)
		{
			QueuedPawns += Pair.Value;
		}
	}

	const int32 Limit = CVarMaxGlobalAIPawns.GetValueOnGameThread();

	int32 NumSpawners = SpawnerPools.Num();
	
	// --- compose message ----------------------------------------------
	Msg = FString::Printf(
		TEXT("[ZodiacAIPawnSubsystem]\n"
				"Tick Interval: %.1f\n"
				"Spawner Counts: %d\n"
				"Pawns Capacity: %d/%d\n"
				"Queued Requests: %d\n"
				"Queued pawns: %d\n"
				),
		TickInterval, NumSpawners, CurrentActive, Limit, QueuedReq, QueuedPawns);
}

void UZodiacAIPawnSubsystem::DisplayDebugMessages()
{
	if (GEngine)
	{
		// --- compose message ----------------------------------------------
		FString Msg;
		MakeDebugMessage(Msg);

		constexpr uint64 MsgKey = 0xA1A1A1A1u;
		GEngine->AddOnScreenDebugMessage(MsgKey, 0.f, FColor::Green, Msg);
	}
}
#endif
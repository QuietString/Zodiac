// the.quiet.string@gmail.com


#include "ZodiacAIPawnSubsystem.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "Character/ZodiacMonster.h"
#include "ZodiacAIPawnSpawner.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAIPawnSubsystem)

void UZodiacAIPawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
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

FZodiacSpawnerPool* UZodiacAIPawnSubsystem::FindSpawnerPool(const AZodiacAIPawnSpawner* Spawner)
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

FZodiacSpawnerPool& UZodiacAIPawnSubsystem::FindOrAddSpawnerPool(const AZodiacAIPawnSpawner* Spawner)
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

void UZodiacAIPawnSubsystem::AddMonsterToPool(const AZodiacAIPawnSpawner* Spawner, const TSubclassOf<AZodiacMonster>& ClassToSpawn, const FZodiacZombieSpawnConfig& SpawnConfig)
{
	UWorld* World = GetWorld();
	if (!World || !ClassToSpawn)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AZodiacMonster* Spawned = World->SpawnActor<AZodiacMonster>(ClassToSpawn, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (!Spawned)
	{
		return;
	}
	
	Spawned->SpawnDefaultController();
	Spawned->Multicast_Sleep();
	
	Spawned->SetSpawnConfig(SpawnConfig);

	// 1) Find or create the pool for that spawner
	FZodiacSpawnerPool& SpawnerPool = FindOrAddSpawnerPool(Spawner);

	// 2) Find or create the sub-pool for that monster class
	auto& [MonsterClass, Instances] = FindOrAddMonsterClassPool(SpawnerPool, ClassToSpawn);

	// 3) Actually store the monster
	Instances.Add(Spawned);
}

void UZodiacAIPawnSubsystem::ReleaseMonsterToPool(const AZodiacAIPawnSpawner* Spawner, AZodiacMonster* Monster)
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
}

AZodiacMonster* UZodiacAIPawnSubsystem::HatchMonsterFromPool(const AZodiacAIPawnSpawner* Spawner, const TSubclassOf<AZodiacMonster>& RequestedClass)
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
	return Monster;
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

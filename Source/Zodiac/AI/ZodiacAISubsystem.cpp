// the.quiet.string@gmail.com


#include "ZodiacAISubsystem.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "Character/ZodiacMonster.h"
#include "Character/ZodiacZombieSpawner.h"

void UZodiacAISubsystem::RegisterSpawner(AZodiacZombieSpawner* Spawner)
{
	if (!Spawner)
	{
		return;
	}

	RegisteredSpawners.AddUnique(Spawner);
}

void UZodiacAISubsystem::UnregisterSpawner(AZodiacZombieSpawner* Spawner)
{
	if (!Spawner)
	{
		return;
	}

	RegisteredSpawners.RemoveAll([Spawner](const TWeakObjectPtr<AZodiacZombieSpawner>& WeakPtr)
	{
		return WeakPtr.Get() == Spawner;
	});
}

void UZodiacAISubsystem::PauseAllMonsters()
{
	for (auto& Spawner : RegisteredSpawners)
	{
		if (Spawner.IsValid())
		{
			for (auto& Monster : Spawner->GetAllMonsters())
			{
				if (Monster)
				{
					if (AAIController* AC = Cast<AAIController>(Monster->GetController()))
					{
						FString Reason = TEXT("Paused by ZodiacAISubsystem");
						AC->BrainComponent->PauseLogic(Reason);
					}
				}
			}
		}
	}
}

void UZodiacAISubsystem::ResumeAllMonsters()
{
	for (auto& Spawner : RegisteredSpawners)
	{
		if (Spawner.IsValid())
		{
			for (auto& Monster : Spawner->GetAllMonsters())
			{
				if (Monster)
				{
					if (AAIController* AC = Cast<AAIController>(Monster->GetController()))
					{
						FString Reason = TEXT("Resumed by ZodiacAISubsystem");
						AC->BrainComponent->ResumeLogic(Reason);
					}
				}
			}
		}
	}
}

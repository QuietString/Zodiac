// the.quiet.string@gmail.com

#include "ZodiacZombieSpawner.h"

#include "ZodiacLogChannels.h"
#include "ZodiacMonster.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacZombieSpawner)

AZodiacZombieSpawner::AZodiacZombieSpawner()
{
}

void AZodiacZombieSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bIsEnabled)
	{
		SpawnMonsters();
	}
}

void AZodiacZombieSpawner::SpawnMonsters()
{
	if (MonstersToSpawn.IsEmpty())
	{
		UE_LOG(LogZodiac, Warning, TEXT("Monsters are not set in MonsterSpawner"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector SpawnOrigin = GetActorLocation();

	for (auto& [MonsterClass, NumberToSpawn] : MonstersToSpawn)
	{
		for (int32 i = 0; i < NumberToSpawn; ++i)
		{
			// Random point within a circle
			float Angle = FMath::RandRange(0.0f, 2.0f * PI);
			float Distance = FMath::RandRange(0.0f, SpawnRadius);
			FVector SpawnLocation = SpawnOrigin + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);

			// Adjust Z position if necessary (e.g., place on the ground)
			FHitResult HitResult;
			FVector Start = SpawnLocation + FVector(0.0f, 0.0f, 500.0f);
			FVector End = SpawnLocation - FVector(0.0f, 0.0f, 1000.0f);
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);

			if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
			{
				SpawnLocation.Z = HitResult.Location.Z;
			}

			// Random rotation
			FRotator SpawnRotation = FRotator(0.0f, FMath::RandRange(-180.0f, 180.0f), 0.0f);

			// Spawn parameters
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			// Spawn the monster
			AZodiacMonster* SpawnedMonster = World->SpawnActor<AZodiacMonster>(MonsterClass, SpawnLocation, SpawnRotation, SpawnParams);

			// Initialize the monster
			if (SpawnedMonster)
			{
				uint8 RandomSeed = FMath::RandRange(0, UINT8_MAX);
				SpawnedMonster->SetSpawnSeed(RandomSeed);
				
				SpawnedMonster->SpawnDefaultController();
			}
		}	
	}
}

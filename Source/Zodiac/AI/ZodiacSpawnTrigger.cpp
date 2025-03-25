// the.quiet.string@gmail.com


#include "ZodiacSpawnTrigger.h"

#include "Character/ZodiacHostCharacter.h"
#include "Character/ZodiacZombieSpawner.h"
#include "Components/BoxComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacSpawnTrigger)

AZodiacSpawnTrigger::AZodiacSpawnTrigger(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Extent(FVector(500.f))
{
	PrimaryActorTick.bCanEverTick = false;

	// Create the box component for overlap detection
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;
	TriggerBox->SetBoxExtent(Extent);
	TriggerBox->SetCollisionProfileName("SpawnTrigger");
}

void AZodiacSpawnTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AZodiacSpawnTrigger::OnOverlapBegin);
    
		bAlreadyTriggered = false;	
	}
}

void AZodiacSpawnTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AZodiacHostCharacter* OverlappingCharacter = Cast<AZodiacHostCharacter>(OtherActor);
	if (!OverlappingCharacter)
	{
		return;
	}

	if (bTriggerOnlyOnce && bAlreadyTriggered)
	{
		return;
	}
	bAlreadyTriggered = true;

	// Trigger spawners
	for (AZodiacZombieSpawner* Spawner : SpawnersToTrigger)
	{
		if (Spawner && Spawner->GetUseTrigger())
		{
			Spawner->SpawnAllMonsters();
		}
	}
}

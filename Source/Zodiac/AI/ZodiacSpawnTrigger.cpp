// the.quiet.string@gmail.com


#include "ZodiacSpawnTrigger.h"

#include "Character/Host/ZodiacHostCharacter.h"
#include "ZodiacAIPawnSpawner.h"
#include "Components/BillboardComponent.h"
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

#if WITH_EDITOR
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (!IsRunningCommandlet() && SpriteComponent)
	{
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture;
			FConstructorStatics()
				: SpriteTexture(TEXT("/Engine/EditorResources/S_TriggerBox"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		SpriteComponent->Sprite = ConstructorStatics.SpriteTexture.Get();
		SpriteComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
		SpriteComponent->bHiddenInGame = true;
		SpriteComponent->SetVisibleFlag(true);
		SpriteComponent->SetupAttachment(RootComponent);
		SpriteComponent->SetAbsolute(false, false, true);
		SpriteComponent->bIsScreenSizeScaled = true;
	}
#endif
}

void AZodiacSpawnTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AZodiacSpawnTrigger::OnOverlapBegin);
    
		bHasTriggered = false;	
	}
}

void AZodiacSpawnTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AZodiacHostCharacter* OverlappingHostCharacter = Cast<AZodiacHostCharacter>(OtherActor);
	if (!OverlappingHostCharacter)
	{
		return;
	}

	if (bTriggerOnlyOnce && bHasTriggered)
	{
		return;
	}

	// Trigger spawners despawn
	for (AZodiacAIPawnSpawner* Spawner : SpawnersToTriggerDespawn)
	{
		Spawner->SendAllMonstersBackToPool();	
	}
	
	// Trigger spawners spawn
	for (AZodiacAIPawnSpawner* Spawner : SpawnersToTrigger)
	{
		Spawner->SpawnAllMonstersFromPool();
	}
	
	bHasTriggered = true;
}

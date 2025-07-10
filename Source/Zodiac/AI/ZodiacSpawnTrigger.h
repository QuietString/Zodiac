// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZodiacSpawnTrigger.generated.h"

class AZodiacAIPawnSpawner;
class UBoxComponent;

UCLASS()
class ZODIAC_API AZodiacSpawnTrigger : public AActor
{
	GENERATED_BODY()

public:
	AZodiacSpawnTrigger(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

public:
	// Which spawner(s) to trigger spawn when the player overlaps this volume
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Spawner Trigger")
	TArray<AZodiacAIPawnSpawner*> SpawnersToTrigger;

	// Which spawner(s) to trigger despawn when the player overlaps this volume
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Spawner Trigger")
	TArray<AZodiacAIPawnSpawner*> SpawnersToTriggerDespawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner Trigger")
	bool bTriggerOnlyOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawner Trigger")
	FVector Extent;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawner Trigger")
	TObjectPtr<UBoxComponent> TriggerBox;

	bool bHasTriggered;

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	UBillboardComponent* SpriteComponent;
#endif
};

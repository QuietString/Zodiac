// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "ZodiacEffectArea.generated.h"

UCLASS()
class ZODIAC_API AZodiacEffectArea : public AActor
{
	GENERATED_BODY()

public:
	AZodiacEffectArea();


protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UBoxComponent* TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffect;
};

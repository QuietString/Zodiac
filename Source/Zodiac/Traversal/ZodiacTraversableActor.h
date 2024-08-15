// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacTraversalTypes.h"
#include "GameFramework/Actor.h"
#include "ZodiacTraversableActor.generated.h"

class USplineComponent;

UCLASS(BlueprintType, Blueprintable)
class ZODIAC_API AZodiacTraversableActor : public AActor
{
	GENERATED_BODY()

public:
	void GetLedgeTransforms(const FVector& HitLocation, const FVector& ActorLocation, FZodiacTraversalCheckResult& CheckResult);

protected:
	USplineComponent* FindLedgeClosestToActor(const FVector& ActorLocation);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	TArray<TObjectPtr<USplineComponent>> Ledges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	TMap<TObjectPtr<USplineComponent>, TObjectPtr<USplineComponent>> OppositeLedges;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float MinLedgeWidth = 60.0f;
};

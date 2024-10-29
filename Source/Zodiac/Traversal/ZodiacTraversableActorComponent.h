// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacTraversalTypes.h"
#include "Components/ActorComponent.h"
#include "ZodiacTraversableActorComponent.generated.h"


class USplineComponent;

UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class ZODIAC_API UZodiacTraversableActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	virtual void BeginPlay() override;
	UZodiacTraversableActorComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	void SpawnSingleSpline(FName StartEdge, FTransform SocketWorldTransform1, FTransform SocketWorldTransform2, USplineComponent*& SplineComp);

	UFUNCTION(BlueprintCallable)
	void SpawnLedgeSplines();

	UFUNCTION(BlueprintCallable)
	void ClearSpawnedSplines();
	
	UFUNCTION(BlueprintCallable)
	TArray<USplineComponent*> GetSplineComponentsOfOwningActor() const;
	UFUNCTION(BlueprintCallable)
	void SetLedges(TArray<USplineComponent*> InLedges);

	UFUNCTION(BlueprintCallable)
	void SetOppositeLedges(TMap<USplineComponent*, USplineComponent*> InOppositeLedges);
	
	void GetLedgeTransforms(const FVector& HitLocation, const FVector& ActorLocation, FZodiacTraversalCheckResult& CheckResult);

protected:
	USplineComponent* FindLedgeClosestToActor(const FVector& ActorLocation);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	TArray<TObjectPtr<USplineComponent>> Ledges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal")
	TMap<USplineComponent*, USplineComponent*> OppositeLedges;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traversal")
	float MinLedgeWidth = 60.0f;
};

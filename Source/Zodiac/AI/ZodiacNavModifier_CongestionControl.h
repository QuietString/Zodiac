// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacNavModifier.h"
#include "ZodiacNavModifier_CongestionControl.generated.h"

UCLASS()
class ZODIAC_API AZodiacNavModifier_CongestionControl : public AZodiacNavModifier
{
	GENERATED_BODY()

public:
	AZodiacNavModifier_CongestionControl(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Congestion")
	TSubclassOf<AActor> CongestionTestClass;

	// Apply congestion area when test actors overlap more than this threshold.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Congestion")
	int32 UpperThreshold = 10;

	// Apply normal area when test actors overlap less than this threshold
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Congestion")
	int32 LowerThreshold = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Congestion")
	TSubclassOf<UNavArea> NormalAreaClass;

	/** Nav area class used when # of overlapping monsters > CongestionThreshold */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Congestion")
	TSubclassOf<UNavArea> CongestedAreaClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Congestion", meta = (ClampMin = 0))
	float TimeToWaitBeforeChange = 0.f;

	/** Whether we are currently using the congested area */
	bool bIsUsingCongestedArea = false;

private:
	float AccumulatedCongestedTime = 0.f;
	
public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

// the.quiet.string@gmail.com


#include "ZodiacNavModifier_CongestionControl.h"

#include "Components/BoxComponent.h"
#include "NavAreas/NavArea.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacNavModifier_CongestionControl)

AZodiacNavModifier_CongestionControl::AZodiacNavModifier_CongestionControl(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickInterval = 0.2f;
}

void AZodiacNavModifier_CongestionControl::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (BoxComponent)
	{
		BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnOverlapBegin);
	}
}

void AZodiacNavModifier_CongestionControl::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	TArray<AActor*> OverlappingActors;
	BoxComponent->GetOverlappingActors(OverlappingActors, CongestionTestClass);
	if (OverlappingActors.Num() > 0)
	{
		SetActorTickEnabled(true);
		
		if ((TimeToWaitBeforeChange <= 0.f) && (OverlappingActors.Num() >= UpperThreshold))
		{
			SetNavArea(CongestedAreaClass);
			bIsUsingCongestedArea = true;
		}
	}
}

void AZodiacNavModifier_CongestionControl::BeginPlay()
{
	Super::BeginPlay();

	bIsUsingCongestedArea = false;
	SetNavArea(NormalAreaClass);
}

void AZodiacNavModifier_CongestionControl::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TArray<AActor*> OverlappingActors;
	BoxComponent->GetOverlappingActors(OverlappingActors, CongestionTestClass);
	int32 OverlapCount = OverlappingActors.Num();

	if (OverlapCount == 0)
	{
		// No one left, go normal and disable tick
		SetNavArea(NormalAreaClass);
		bIsUsingCongestedArea = false;
		SetActorTickEnabled(false);
		return;
	}

	if (!bIsUsingCongestedArea)
	{
		if (OverlapCount >= UpperThreshold)
		{
			AccumulatedCongestedTime += DeltaSeconds;
			if (AccumulatedCongestedTime >= TimeToWaitBeforeChange)
			{
				SetNavArea(CongestedAreaClass);
				bIsUsingCongestedArea = true;
				AccumulatedCongestedTime = 0.f;
			}
		}
		else
		{
			AccumulatedCongestedTime = 0.f;
		}
	}
	else
	{
		// If we are already congested, see if we can revert
		if (OverlapCount <= LowerThreshold)
		{
			SetNavArea(NormalAreaClass);
			bIsUsingCongestedArea = false;
			AccumulatedCongestedTime = 0.f;
		}
	}
}

#if WITH_EDITOR

void AZodiacNavModifier_CongestionControl::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetNavArea(NormalAreaClass);
}

void AZodiacNavModifier_CongestionControl::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(AZodiacNavModifier_CongestionControl, NormalAreaClass))
	{
		SetNavArea(NormalAreaClass);
	}
}
#endif
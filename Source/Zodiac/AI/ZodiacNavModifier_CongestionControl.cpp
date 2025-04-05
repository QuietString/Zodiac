// the.quiet.string@gmail.com


#include "ZodiacNavModifier_CongestionControl.h"

#include "Components/BoxComponent.h"
#include "NavAreas/NavArea.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacNavModifier_CongestionControl)

AZodiacNavModifier_CongestionControl::AZodiacNavModifier_CongestionControl(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.25f;
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
	GetOverlappingActors(OverlappingActors, CongestionTestClass);

	if (OverlappingActors.Num() >= UpperThreshold)
	{
		SetNavArea(CongestedAreaClass);
		bIsUsingCongestedArea = true;
		SetActorTickEnabled(true);
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
	
	if (!BoxComponent || !NavModifierComp)
	{
		return;
	}

	// Check overlap by tick since OnComponentEndOverlap event is not reliable.
	TArray<AActor*> OverlappingActors;
	BoxComponent->GetOverlappingActors(OverlappingActors, CongestionTestClass);
	const int32 OverlapCounts = OverlappingActors.Num();
	
	if (bIsUsingCongestedArea && OverlapCounts <= LowerThreshold)
	{
		// Switch back to normal area
		SetNavArea(NormalAreaClass);
		bIsUsingCongestedArea = false;
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
// the.quiet.string@gmail.com


#include "ZodiacNavModifier_CongestionControl.h"

#include "Components/BoxComponent.h"
#include "NavAreas/NavArea.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacNavModifier_CongestionControl)

AZodiacNavModifier_CongestionControl::AZodiacNavModifier_CongestionControl(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
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

	TArray<AActor*> OverlappingActors;
	BoxComponent->GetOverlappingActors(OverlappingActors, CongestionTestClass);
	const int32 OverlapCounts = OverlappingActors.Num();

	// Decide if we should be congested or not:
	if (!bIsUsingCongestedArea && OverlapCounts >= UpperThreshold)
	{
		// Switch to congested area
		SetNavArea(CongestedAreaClass);
		bIsUsingCongestedArea = true;
	}
	else if (bIsUsingCongestedArea && OverlapCounts < LowerThreshold)
	{
		// Switch back to normal area
		SetNavArea(NormalAreaClass);
		bIsUsingCongestedArea = false;
	}
}

#if WITH_EDITOR
void AZodiacNavModifier_CongestionControl::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(AZodiacNavModifier_CongestionControl, NormalAreaClass))
	{
		SetNavArea(NormalAreaClass);
	}
}
#endif
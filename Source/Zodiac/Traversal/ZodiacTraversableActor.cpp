// the.quiet.string@gmail.com


#include "ZodiacTraversableActor.h"

#include "Components/SplineComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacTraversableActor)

void AZodiacTraversableActor::GetLedgeTransforms(const FVector& HitLocation, const FVector& ActorLocation, IN OUT FZodiacTraversalCheckResult& CheckResult)
{
	// Find the ledge closest to the actors location.
	TObjectPtr<USplineComponent> FrontLedge = FindLedgeClosestToActor(ActorLocation);
	if (!FrontLedge)
	{
		CheckResult.bHasFrontLedge = false;
		return;
	}

	// Make sure the ledge is wide enough. If not, the front ledge will not be valid.
	if (FrontLedge->GetSplineLength() < MinLedgeWidth)
	{
		CheckResult.bHasFrontLedge = false;
		return;
	}
	
	// Get the closest point on the ledge to the actor, and clamp the location so that it can't be too close to the end of the ledge.
	// This prevents the character from floating if they traverse near a corner.
	// The clamped location will always be half the "min ledge width" from the edge.
	// If the min ledge width is 60 units, the ledge location will always be at least 30 units away from a corner.
	FVector HitLocal = FrontLedge->FindLocationClosestToWorldLocation(HitLocation, ESplineCoordinateSpace::Local);
	float Distance = FrontLedge->GetDistanceAlongSplineAtLocation(HitLocal, ESplineCoordinateSpace::Local);
	float MinDistance = MinLedgeWidth / 2;
	float MaxDistance = FrontLedge->GetSplineLength() - (MinLedgeWidth / 2);
	float ClampedDistance = FMath::Clamp(Distance, MinDistance, MaxDistance);
	FTransform LedgeTransform = FrontLedge->GetTransformAtDistanceAlongSpline(ClampedDistance, ESplineCoordinateSpace::World);
	CheckResult.bHasFrontLedge = true;
	CheckResult.FrontLedgeLocation = LedgeTransform.GetLocation();
	CheckResult.FrontLedgeNormal = LedgeTransform.GetRotation().GetUpVector();

	TObjectPtr<USplineComponent> BackLedge = OppositeLedges[FrontLedge];
	if (!BackLedge)
	{
		CheckResult.bHasBackLedge = false;
		return;
	}

	FTransform BackLedgeTransform = BackLedge->FindTransformClosestToWorldLocation(CheckResult.FrontLedgeLocation, ESplineCoordinateSpace::World);
	CheckResult.bHasBackLedge = true;
	CheckResult.BackLedgeLocation = BackLedgeTransform.GetLocation();
	CheckResult.BackLedgeNormal = BackLedgeTransform.GetRotation().GetUpVector();

	return;
}

USplineComponent* AZodiacTraversableActor::FindLedgeClosestToActor(const FVector& ActorLocation)
{
	if (Ledges.IsEmpty())
	{
		return nullptr;
	}

	float ClosestDistance = BIG_NUMBER;
	int32 ClosestIndex = INDEX_NONE;
	int32 CurrentIndex = 0;
	for (USplineComponent* Ledge : Ledges)
	{
		FVector LedgeLocation = Ledge->FindLocationClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World);
		FVector LocationOffset = Ledge->FindUpVectorClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World) * 10.0f;
		FVector LedgeLocationWithOffset = LedgeLocation + LocationOffset;
		
		float CurrentDistance = FVector::Distance(ActorLocation, LedgeLocationWithOffset); 
		if (CurrentDistance < ClosestDistance)
		{
			ClosestDistance = CurrentDistance;
			ClosestIndex = CurrentIndex;
		} 

		CurrentIndex++;
	}

	return Ledges[ClosestIndex];
}

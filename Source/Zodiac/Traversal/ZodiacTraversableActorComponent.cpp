// the.quiet.string@gmail.com


#include "ZodiacTraversableActorComponent.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/StaticMeshSocket.h"
#include "ZodiacTraversalTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacTraversableActorComponent)

namespace ZodiacTraversable
{
	static FName Tag(TEXT("Traversable"));
	
	static FName Socket1(TEXT("Edge"));
	static FName Socket2(TEXT("Edge_1"));
	static FName Socket3(TEXT("Edge_2"));
	static FName Socket4(TEXT("Edge_3"));

	static FName Socket5(TEXT("Edge_4"));
	static FName Socket6(TEXT("Edge_5"));
	static FName Socket7(TEXT("Edge_6"));
	static FName Socket8(TEXT("Edge_7"));
	
	static TMap<FName, FName> Edges = {
		{ Socket1, Socket2 },
		{ Socket2, Socket3 },
		{ Socket3, Socket4 },
		{ Socket4, Socket1 },

		{ Socket5, Socket6 },
		{ Socket6, Socket7 },
		{ Socket7, Socket8 },
		{ Socket8, Socket1 }
	};

	// use the first socket of each edge as the name of the edge.
	// every socket has same rotation, aligned with first edge normal.
	// to make each spline face outward from a owning mesh, edges along counterclockwise should be rotation 90 degrees by y axis
	static TMap<FName, float> RotationAmount = {
		{ Socket1, 0 },
		{ Socket2, -90 },
		{ Socket3, -180 },
		{ Socket4, -270 },

		{ Socket5, 0 },
		{ Socket6, -90 },
		{ Socket7, -180 },
		{ Socket8, -270 },
	};
	
	static TMap<FName, FName> OppositeEdges = {
		{ Socket1, Socket3 },
		{ Socket3, Socket1 },
		{ Socket2, Socket4 },
		{ Socket4, Socket2 },

		{ Socket5, Socket7 },
		{ Socket7, Socket5 },
		{ Socket6, Socket8 },
		{ Socket8, Socket6 },
	};
}

void UZodiacTraversableActorComponent::BeginPlay()
{
	Super::BeginPlay();

	SpawnLedgeSplines();
}

UZodiacTraversableActorComponent::UZodiacTraversableActorComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZodiacTraversableActorComponent::SpawnSingleSpline(FName StartEdge, FTransform SocketWorldTransform1, FTransform SocketWorldTransform2, USplineComponent*& SplineComp)
{
	FVector Point1 = SocketWorldTransform1.GetLocation();
	FVector Point2 = SocketWorldTransform2.GetLocation();

	SplineComp = NewObject<USplineComponent>(this, USplineComponent::StaticClass());
	SplineComp->RegisterComponent();
	SplineComp->SetMobility(EComponentMobility::Movable);
	SplineComp->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	SplineComp->ClearSplinePoints();

	FQuat OriginalRotation = SocketWorldTransform1.GetRotation();
	float RotationAmount = ZodiacTraversable::RotationAmount[StartEdge];
	FQuat YAxisRotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(RotationAmount));
	FQuat RotatedRotation = YAxisRotation * OriginalRotation;
						
	// Add two spline points with location and rotation
	SplineComp->AddSplinePointAtIndex(Point1, 0, ESplineCoordinateSpace::World, false);
	SplineComp->SetSplinePointType(0, ESplinePointType::Linear, false);
	SplineComp->SetTangentAtSplinePoint(0, RotatedRotation.GetForwardVector(), ESplineCoordinateSpace::World, false);
	SplineComp->SetUpVectorAtSplinePoint(0, RotatedRotation.GetUpVector(), ESplineCoordinateSpace::World, false);
						
	SplineComp->AddSplinePointAtIndex(Point2, 1, ESplineCoordinateSpace::World, false);
	SplineComp->SetSplinePointType(1, ESplinePointType::Linear, false);
	SplineComp->SetTangentAtSplinePoint(1, RotatedRotation.GetForwardVector(), ESplineCoordinateSpace::World, false);
	SplineComp->SetUpVectorAtSplinePoint(1, RotatedRotation.GetUpVector(), ESplineCoordinateSpace::World, false);

	SplineComp->UpdateSpline();

#if WITH_EDITOR
	if (ZodiacConsoleVariables::CVarTraversalDrawDebug.GetValueOnAnyThread())
	{
		// Calculate the end points for each axis arrow
		FVector MidLocation = (Point1 + Point2) / 2;
		FVector ForwardEnd = MidLocation + RotatedRotation.GetForwardVector() * 30;
		FVector UpEnd = MidLocation + RotatedRotation.GetUpVector() * 30;
		FVector RightEnd = MidLocation + RotatedRotation.GetRightVector() * 30;

		// Draw lines representing each axis
		DrawDebugLine(GetWorld(), MidLocation, ForwardEnd, FColor::Red, true);
		DrawDebugLine(GetWorld(), MidLocation, UpEnd, FColor::Blue, true);
		DrawDebugLine(GetWorld(), MidLocation, RightEnd, FColor::Green, true);
	}
#endif
}

void UZodiacTraversableActorComponent::SpawnLedgeSplines()
{
	Ledges.Empty();
	OppositeLedges.Empty();
	
	TArray<UActorComponent*> Components = GetOwner()->GetComponents().Array();
	for (auto Comp : Components)
	{
		// spawn splines for instanced meshes
		if (UInstancedStaticMeshComponent* InstMesh = Cast<UInstancedStaticMeshComponent>(Comp))
		{
			int32 InstanceCount = InstMesh->GetInstanceCount();
			for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount; ++InstanceIndex)
			{
				TMap<FName, USplineComponent*> NameToSpline;
				TMap<USplineComponent*, FName> SplineToName;
				
				for (const auto& [StartEdge, EndEdge] : ZodiacTraversable::Edges)
				{
					const UStaticMeshSocket* Socket1 = InstMesh->GetStaticMesh()->FindSocket(StartEdge);
					const UStaticMeshSocket* Socket2 = InstMesh->GetStaticMesh()->FindSocket(EndEdge);
					if (Socket1 && Socket2)
					{
						FTransform SocketTransform1;
						SocketTransform1 = InstMesh->GetSocketTransform(StartEdge, RTS_Component);
						
						FTransform SocketTransform2;
						SocketTransform2 = InstMesh->GetSocketTransform(EndEdge, RTS_Component);

						FTransform InstanceTransform;
						InstMesh->GetInstanceTransform(InstanceIndex, InstanceTransform, true);
						
						FTransform SocketWorldTransform1 = SocketTransform1 * InstanceTransform;
						FTransform SocketWorldTransform2 = SocketTransform2 * InstanceTransform;

						USplineComponent* SplineComp;
						SpawnSingleSpline(StartEdge, SocketWorldTransform1, SocketWorldTransform2, SplineComp);
						
						NameToSpline.Add(StartEdge, SplineComp);
						SplineToName.Add(SplineComp, StartEdge);
						Ledges.Add(SplineComp);
					}
				}
				
				if (!SplineToName.IsEmpty())
				{
					for (auto& [Spline, Name] : SplineToName)
					{
						FName OppositeName = ZodiacTraversable::OppositeEdges[Name];
						if (NameToSpline.Contains(OppositeName))
						{
							USplineComponent* Opposite = NameToSpline[OppositeName]; 
							OppositeLedges.Add(Spline, Opposite);
							OppositeLedges.Add(Opposite, Spline);
						}
					}	
				}
			}
		}
		// spawn splines for static meshes
		else if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Comp))
		{
			TMap<FName, USplineComponent*> NameToSpline;
			TMap<USplineComponent*, FName> SplineToName;
				
			for (const auto& [StartEdge, EndEdge] : ZodiacTraversable::Edges)
			{
				const UStaticMeshSocket* Socket1 = Mesh->GetStaticMesh()->FindSocket(StartEdge);
				const UStaticMeshSocket* Socket2 = Mesh->GetStaticMesh()->FindSocket(EndEdge);
				if (Socket1 && Socket2)
				{
					FTransform SocketTransform1;
					SocketTransform1 = Mesh->GetSocketTransform(StartEdge, RTS_World);
						
					FTransform SocketTransform2;
					SocketTransform2 = Mesh->GetSocketTransform(EndEdge, RTS_World);

					USplineComponent* SplineComp;
					SpawnSingleSpline(StartEdge, SocketTransform1, SocketTransform2, SplineComp);
						
					NameToSpline.Add(StartEdge, SplineComp);
					SplineToName.Add(SplineComp, StartEdge);
					Ledges.Add(SplineComp);
				}
			}
				
			if (!SplineToName.IsEmpty())
			{
				for (auto& [Spline, Name] : SplineToName)
				{
					FName OppositeName = ZodiacTraversable::OppositeEdges[Name];
					if (NameToSpline.Contains(OppositeName))
					{
						USplineComponent* Opposite = NameToSpline[OppositeName]; 
						OppositeLedges.Add(Spline, Opposite);
						OppositeLedges.Add(Opposite, Spline);
					}
				}	
			}
		}
	}
}

void UZodiacTraversableActorComponent::ClearSpawnedSplines()
{
	if (!Ledges.IsEmpty())
	{
		for (USplineComponent* Spline : Ledges)
		{
			Spline->DestroyComponent();
		}

		Ledges.Empty();	
	}
}

TArray<USplineComponent*> UZodiacTraversableActorComponent::GetSplineComponentsOfOwningActor() const
{
	TArray<UActorComponent*> Components;
	GetOwner()->GetComponents(USplineComponent::StaticClass(), Components);
	
	for (UActorComponent* Comp : Components)
	{
		if (USplineComponent* SplineComponent = Cast<USplineComponent>(Comp))
		{
		}
	}

	return Ledges;
}

void UZodiacTraversableActorComponent::SetLedges(TArray<USplineComponent*> InLedges)
{
	if (!InLedges.IsEmpty())
	{
		Ledges = InLedges;	
	}
}

void UZodiacTraversableActorComponent::SetOppositeLedges(TMap<USplineComponent*, USplineComponent*> InOppositeLedges)
{
	if (!InOppositeLedges.IsEmpty())
	{
		OppositeLedges = InOppositeLedges;	
	}
}

void UZodiacTraversableActorComponent::GetLedgeTransforms(const FVector& HitLocation, const FVector& ActorLocation,
                                                          FZodiacTraversalCheckResult& CheckResult)
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

	if (!OppositeLedges.Contains(FrontLedge))
	{
		CheckResult.bHasBackLedge = false;
		return;
	}
	
	TObjectPtr<USplineComponent> BackLedge = OppositeLedges[FrontLedge];
	FTransform BackLedgeTransform = BackLedge->FindTransformClosestToWorldLocation(CheckResult.FrontLedgeLocation, ESplineCoordinateSpace::World);
	CheckResult.bHasBackLedge = true;
	CheckResult.BackLedgeLocation = BackLedgeTransform.GetLocation();
	CheckResult.BackLedgeNormal = BackLedgeTransform.GetRotation().GetUpVector();
}

USplineComponent* UZodiacTraversableActorComponent::FindLedgeClosestToActor(const FVector& ActorLocation)
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

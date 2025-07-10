// the.quiet.string@gmail.com


#include "ZodiacTraversalNavLink.h"

#include "ZodiacTraversalComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacTraversalNavLink)

AZodiacTraversalNavLink::AZodiacTraversalNavLink(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bSmartLinkIsRelevant = true;
}

void AZodiacTraversalNavLink::CacheTraversalResults()
{
}

void AZodiacTraversalNavLink::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	OnSmartLinkReached.AddDynamic(this, &ThisClass::OnLinkReached);
}

void AZodiacTraversalNavLink::BeginPlay()
{
	Super::BeginPlay();
}

void AZodiacTraversalNavLink::OnLinkReached(AActor* MovingActor, const FVector& DestinationPoint)
{
	if (UZodiacTraversalComponent* TraversalComponent = MovingActor->FindComponentByClass<UZodiacTraversalComponent>())
	{
	}
}

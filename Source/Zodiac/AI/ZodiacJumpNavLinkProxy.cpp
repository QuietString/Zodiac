// the.quiet.string@gmail.com


#include "ZodiacJumpNavLinkProxy.h"

#include "NavigationSystem.h"

AZodiacJumpNavLinkProxy::AZodiacJumpNavLinkProxy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bSmartLinkIsRelevant = true;
}

void AZodiacJumpNavLinkProxy::BeginPlay()
{
	Super::BeginPlay();
}

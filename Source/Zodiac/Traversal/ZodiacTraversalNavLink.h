// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacTraversalTypes.h"
#include "Navigation/NavLinkProxy.h"
#include "ZodiacTraversalNavLink.generated.h"

UCLASS(Blueprintable)
class ZODIAC_API AZodiacTraversalNavLink : public ANavLinkProxy
{
	GENERATED_BODY()

public:
	AZodiacTraversalNavLink(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	
	// Caches traversal check results (from start-to-end and vice versa).
	UFUNCTION(BlueprintCallable, Category="Traversal")
	void CacheTraversalResults();

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnLinkReached(AActor* MovingActor, const FVector& DestinationPoint);
};

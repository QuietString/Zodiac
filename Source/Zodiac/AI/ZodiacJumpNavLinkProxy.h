// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavLinkProxy.h"
#include "ZodiacJumpNavLinkProxy.generated.h"


UCLASS(BlueprintType, Blueprintable)
class ZODIAC_API AZodiacJumpNavLinkProxy : public ANavLinkProxy
{
	GENERATED_BODY()

public:
	AZodiacJumpNavLinkProxy(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ZodiacPawn.generated.h"

UCLASS()
class ZODIAC_API AZodiacPawn : public APawn	
{
	GENERATED_BODY()

public:
	AZodiacPawn();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};

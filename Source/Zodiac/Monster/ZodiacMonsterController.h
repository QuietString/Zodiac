// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "ZodiacMonsterController.generated.h"

UCLASS()
class ZODIAC_API AZodiacMonsterController : public AAIController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;

protected:
	void OnStun(FGameplayTag CallbackTag, int32 NewCount);
};

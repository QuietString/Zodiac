// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_RunEQS.h"
#include "BTService_RunEQSConditional.generated.h"

/**
 * Run EQS only if a given Blackboard bool key is true.
 */
UCLASS()
class ZODIAC_API UBTService_RunEQSConditional : public UBTService_RunEQS
{
	GENERATED_BODY()

public:
	UBTService_RunEQSConditional(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** Blackboard key that controls whether we should run EQS. 
		Must be a bool key in the blackboard. */
	UPROPERTY(EditAnywhere, Category = "Condition")
	FBlackboardKeySelector ShouldRunEQSKey;

	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInverseCondition;
	
	//~ Begin UBTService Interface
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	//~ End UBTService Interface
};

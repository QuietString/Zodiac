// the.quiet.string@gmail.com


#include "BTService_RunEQSConditional.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BTService_RunEQSConditional)

UBTService_RunEQSConditional::UBTService_RunEQSConditional(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Run EQS (Conditional)";
}

void UBTService_RunEQSConditional::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!OwnerComp.GetBlackboardComponent())
	{
		Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
		return;
	}

	// Check the condition Blackboard bool
	bool bShouldRunEQS = OwnerComp.GetBlackboardComponent()->GetValueAsBool(ShouldRunEQSKey.SelectedKeyName);
	if (bInverseCondition)
	{
		bShouldRunEQS = !bShouldRunEQS;
	}

	if (!bShouldRunEQS)
	{
		// If the condition has turned FALSE, abort any running query:
		FBTEQSServiceMemory* MyMemory = CastInstanceNodeMemory<FBTEQSServiceMemory>(NodeMemory);
		if (MyMemory && MyMemory->RequestID != INDEX_NONE)
		{
			if (UWorld* World = OwnerComp.GetWorld())
			{
				if (UEnvQueryManager* EQSManager = UEnvQueryManager::GetCurrent(World))
				{
					EQSManager->AbortQuery(MyMemory->RequestID);
				}
			}
			MyMemory->RequestID = INDEX_NONE;
		}

		// Skip running EQS this tick
		return;
	}

	// If bShouldRunEQS is true, run the parent's normal EQS logic:
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
}
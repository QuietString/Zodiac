// the.quiet.string@gmail.com


#include "ZodiacMonsterController.h"

#include "BrainComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacMonster.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"


void AZodiacMonsterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AZodiacMonster* Monster = Cast<AZodiacMonster>(InPawn))
	{
		if (UZodiacAbilitySystemComponent* ZodiacASC = Monster->GetZodiacAbilitySystemComponent())
		{
			ZodiacASC->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStun);
		}
	}
}

void AZodiacMonsterController::OnStun(FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		BrainComponent->StopLogic(TEXT("Stunned"));
	}
	else
	{
		BrainComponent->RestartLogic();
	}
}

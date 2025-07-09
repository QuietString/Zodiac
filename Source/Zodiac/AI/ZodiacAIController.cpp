// the.quiet.string@gmail.com


#include "ZodiacAIController.h"

#include "Character/ZodiacCharacter.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Navigation/PathFollowingComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAIController)

AZodiacAIController::AZodiacAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = false;
}

UAbilitySystemComponent* AZodiacAIController::GetAbilitySystemComponent() const
{
	if (AZodiacCharacter* ZodiacCharacter = GetPawn<AZodiacCharacter>())
	{
		return ZodiacCharacter->GetAbilitySystemComponent();
	}
	
	return nullptr;
}

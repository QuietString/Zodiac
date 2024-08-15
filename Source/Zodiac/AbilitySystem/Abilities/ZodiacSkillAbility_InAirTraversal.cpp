// the.quiet.string@gmail.com


#include "ZodiacSkillAbility_InAirTraversal.h"

#include "Character/ZodiacCharacterMovementComponent.h"
#include "Character/ZodiacHostCharacter.h"
#include "Player/ZodiacPlayerController.h"
#include "Traversal/ZodiacTraversalComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacSkillAbility_InAirTraversal)

void UZodiacSkillAbility_InAirTraversal::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (AZodiacHostCharacter* HostCharacter = GetZodiacHostCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* CharMovComp = HostCharacter->GetCharacterMovement())
		{
			if ((CharMovComp->CustomMovementMode != MOVE_Traversal) && CharMovComp->IsFalling())
			{
				bool bIsInAir = true;
				UZodiacTraversalComponent* TraversalComponent = HostCharacter->FindComponentByClass<UZodiacTraversalComponent>();
				TraversalComponent->TryTraversalActionFromAbility(bIsInAir);
			}
		}
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
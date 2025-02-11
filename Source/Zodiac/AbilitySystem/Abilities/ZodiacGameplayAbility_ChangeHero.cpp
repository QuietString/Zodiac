// the.quiet.string@gmail.com


#include "ZodiacGameplayAbility_ChangeHero.h"

#include "Character/ZodiacHostCharacter.h"

void UZodiacGameplayAbility_ChangeHero::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                        const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (SlotIndex != INDEX_NONE)
	{
		if (AZodiacHostCharacter* HostCharacter = GetZodiacHostCharacterFromActorInfo())
		{
			HostCharacter->ChangeHero(SlotIndex);
		}
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

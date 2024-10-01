// the.quiet.string@gmail.com


#include "ZodiacHeroAbility_ChangeHero.h"

#include "AbilitySystemComponent.h"
#include "Character/ZodiacHostCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAbility_ChangeHero)

void UZodiacHeroAbility_ChangeHero::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                        const FGameplayEventData* TriggerEventData)
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

// the.quiet.string@gmail.com


#include "ZodiacGameplayAbility_HoldAndRelease.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

void UZodiacGameplayAbility_HoldAndRelease::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                            const FGameplayEventData* TriggerEventData)
{
	UAbilityTask_WaitInputRelease* WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	WaitInputRelease->OnRelease.AddDynamic(this, &ThisClass::OnInputRelease);
	WaitInputRelease->Activate();
		
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UZodiacGameplayAbility_HoldAndRelease::OnInputRelease_Implementation(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


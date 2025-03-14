// the.quiet.string@gmail.com


#include "ZodiacHostAbility_Sprint.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Character/ZodiacHostCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHostAbility_Sprint)

UZodiacHostAbility_Sprint::UZodiacHostAbility_Sprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UZodiacHostAbility_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	WaitInputRelease->OnRelease.AddDynamic(this, &ThisClass::OnInputRelease);
	WaitInputRelease->Activate();
	
	if (AZodiacHostCharacter* ZodiacCharacter = GetZodiacHostCharacterFromActorInfo())
	{
		ZodiacCharacter->ToggleSprint(true);
	}
}

void UZodiacHostAbility_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	WaitInputRelease->OnRelease.Clear();

	if (AZodiacHostCharacter* ZodiacCharacter = GetZodiacHostCharacterFromActorInfo())
	{
		ZodiacCharacter->ToggleSprint(false);
	}
}

void UZodiacHostAbility_Sprint::OnInputRelease(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
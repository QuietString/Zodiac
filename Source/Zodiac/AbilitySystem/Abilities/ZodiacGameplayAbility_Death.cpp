// the.quiet.string@gmail.com


#include "ZodiacGameplayAbility_Death.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Character/ZodiacHealthComponent.h"
#include "Character/ZodiacHostCharacter.h"

UZodiacGameplayAbility_Death::UZodiacGameplayAbility_Death(const FObjectInitializer& ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	bAutoStartDeath = true;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = ZodiacGameplayTags::GameplayEvent_Death;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UZodiacGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);

	UZodiacAbilitySystemComponent* ZodiacASC = CastChecked<UZodiacAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

	FGameplayTagContainer AbilityTypesToIgnore;
	AbilityTypesToIgnore.AddTag(ZodiacGameplayTags::Ability_Behavior_SurvivesDeath);

	// Cancel all abilities and block others from starting.
	ZodiacASC->CancelAbilities(nullptr, &AbilityTypesToIgnore, this);

	SetCanBeCanceled(false);

	// if (!ChangeActivationGroup(EZodiacAbilityActivationGroup::Exclusive_Blocking))
	// {
	// 	UE_LOG(LogZodiacAbilitySystem, Error, TEXT("UZodiacGameplayAbility_Death::ActivateAbility: Ability [%s] failed to change activation group to blocking."), *GetName());
	// }

	if (bAutoStartDeath)
	{
		StartDeath();
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UZodiacGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UZodiacGameplayAbility_Death::StartDeath()
{
	if (AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetOwningActorFromActorInfo()))
	{
		if (UZodiacHealthComponent* HealthComponent = HostCharacter->GetCurrentHeroHealthComponent())
		{
			if (HealthComponent->GetDeathState() == EZodiacDeathState::NotDead)
			{
				HealthComponent->StartDeath();
			}
		}
	}
}

void UZodiacGameplayAbility_Death::FinishDeath()
{
	if (AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetOwningActorFromActorInfo()))
	{
		if (UZodiacHealthComponent* HealthComponent = HostCharacter->GetCurrentHeroHealthComponent())
		{
			if (HealthComponent->GetDeathState() == EZodiacDeathState::DeathStarted)
			{
				HealthComponent->FinishDeath();
			}
		}
	}
}

// the.quiet.string@gmail.com


#include "ZodiacGameplayAbility_Death.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Character/ZodiacHealthComponent.h"
#include "Character/ZodiacCharacter.h"

UZodiacGameplayAbility_Death::UZodiacGameplayAbility_Death(const FObjectInitializer& ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	bAutoStartDeath = true;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = ZodiacGameplayTags::Event_Death;
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
	// Always try to finish the death when the ability ends in case the ability doesn't.
	// This won't do anything if the death hasn't been started.
	FinishDeath();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UZodiacGameplayAbility_Death::StartDeath()
{
	if (AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(GetOwningActorFromActorInfo()))
	{
		if (UZodiacHealthComponent* HealthComponent = ZodiacCharacter->GetHealthComponent())
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
	if (AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(GetOwningActorFromActorInfo()))
	{
		if (UZodiacHealthComponent* HealthComponent = ZodiacCharacter->GetHealthComponent())
		{
			if (HealthComponent->GetDeathState() == EZodiacDeathState::DeathStarted)
			{
				HealthComponent->FinishDeath();
			}
		}
	}
}

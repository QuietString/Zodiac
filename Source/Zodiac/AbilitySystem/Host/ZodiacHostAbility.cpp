// the.quiet.string@gmail.com


#include "ZodiacHostAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"

namespace FAbilitySystemTweaks
{
	int ClearAbilityTimers = 1;
	FAutoConsoleVariableRef CVarClearAbilityTimers(TEXT("AbilitySystem.ClearAbilityTimers"), FAbilitySystemTweaks::ClearAbilityTimers, TEXT("Whether to call ClearAllTimersForObject as part of EndAbility call"), ECVF_Default);
}

int32 FScopedCanActivateAbilityLogEnabler::LogEnablerCounter = 0;

void UZodiacHostAbility::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	UAbilitySystemComponent* Comp = ActorInfo->AbilitySystemComponent.Get();

	// Block and cancel hero abilities too.
	Comp->ApplyAbilityBlockAndCancelTags(GetAssetTags(), nullptr, true, BlockAbilitiesWithTag_Hero, true, CancelAbilitiesWithTag_Hero);
}

void UZodiacHostAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			UE_LOG(LogAbilitySystem, Verbose, TEXT("Attempting to end Host Ability %s but ScopeLockCount was greater than 0, adding end to the WaitingToExecute Array"), *GetName());
			return;
		}
        
		if (GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			bIsAbilityEnding = true;
		}
		
		// Protect against blueprint causing us to EndAbility already
		if (bIsActive == false && GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			return;
		}

		if (UAbilitySystemComponent* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
		{
			if (IsBlockingOtherAbilities())
			{
				// If we're still blocking other abilities, cancel now
				AbilitySystemComponent->ApplyAbilityBlockAndCancelTags(GetAssetTags(), nullptr, false, BlockAbilitiesWithTag_Hero, false, CancelAbilitiesWithTag_Hero);
			}
		}
	}
}

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
	// Cache whether we were still active before calling the parent
	bool bWasActiveBefore = bIsActive;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (bWasActiveBefore && !bIsActive)
	{
		if (UAbilitySystemComponent* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
		{
			if (IsBlockingOtherAbilities())
			{
				AbilitySystemComponent->ApplyAbilityBlockAndCancelTags(GetAssetTags(), nullptr, false, BlockAbilitiesWithTag_Hero, false, CancelAbilitiesWithTag_Hero);
			}
		}
	}
}

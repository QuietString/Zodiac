// the.quiet.string@gmail.com


#include "ZodiacGameplayAbility_Traversal.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacLogChannels.h"
#include "Character/ZodiacCharacter.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "Traversal/ZodiacTraversalComponent.h"
#include "Traversal/ZodiacTraversalTypes.h"
#include "GameplayAbilities/Public/Abilities/Tasks/AbilityTask_WaitDelay.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacGameplayAbility_Traversal)

UZodiacGameplayAbility_Traversal::UZodiacGameplayAbility_Traversal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = ZodiacGameplayTags::Event_Ability_Traversal;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

bool UZodiacGameplayAbility_Traversal::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                            const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
		{
			if (UZodiacTraversalComponent* TraversalComponent = ZodiacCharacter->FindComponentByClass<UZodiacTraversalComponent>())
			{
				// If this ability is triggered from Jump Ability, it could already have a cached result.
				TraversalComponent->ClearCheckResultCache();
			}
		}
		return false;
	}

	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (UZodiacTraversalComponent* TraversalComponent = ZodiacCharacter->FindComponentByClass<UZodiacTraversalComponent>())
		{
			FGameplayTag FailReason;
			FVector FrontLedgeLocation;
			FVector FrontLedgeNormal;
			AActor* BlockingActor = nullptr;
			bool Result = TraversalComponent->CanTraversalAction(FailReason, FrontLedgeLocation, FrontLedgeNormal, BlockingActor);

#if WITH_EDITOR
			if (!Result && ZodiacConsoleVariables::CVarTraversalDrawDebug.GetValueOnAnyThread())
			{
				UE_LOG(LogZodiacTraversal, Log, TEXT("Traversal Failed Reason: %s"), *FailReason.ToString());
			}
#endif
			return Result;
		}
	}

	return false;
}

void UZodiacGameplayAbility_Traversal::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                         const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (UZodiacTraversalComponent* TraversalComponent = ZodiacCharacter->FindComponentByClass<UZodiacTraversalComponent>())
		{
			TraversalComponent->OnTraversalFinished.BindUObject(this, &ThisClass::OnTraversalFinished);

			FZodiacTraversalCheckResult CheckResult = TraversalComponent->GetCachedCheckResult();
			TraversalComponent->Server_PerformTraversalAction(CheckResult);

			// Automatically ends ability after some time in case.
			UAbilityTask_WaitDelay* WaitDelay =  UAbilityTask_WaitDelay::WaitDelay(this, 5.0f);
			WaitDelay->OnFinish.AddDynamic(this, &ThisClass::OnTraversalFinished);
			WaitDelay->Activate();
		}
	}
}

void UZodiacGameplayAbility_Traversal::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (UZodiacTraversalComponent* TraversalComponent = ZodiacCharacter->FindComponentByClass<UZodiacTraversalComponent>())
		{
			TraversalComponent->ClearCheckResultCache();
		}
	}
}

void UZodiacGameplayAbility_Traversal::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (UZodiacTraversalComponent* TraversalComponent = ZodiacCharacter->FindComponentByClass<UZodiacTraversalComponent>())
		{
			FZodiacTraversalCheckResult CheckResult = TraversalComponent->GetCachedCheckResult();
			TraversalComponent->ClearPerformResult();
		}

		if (!bHasEndedByNotify)
		{
			if (UZodiacCharacterMovementComponent* ZodiacCharMoveComp = Cast<UZodiacCharacterMovementComponent>(ZodiacCharacter->GetCharacterMovement()))
			{
				ZodiacCharMoveComp->SetMovementModeToDefault();
			}
		}
	}

	bHasEndedByNotify = false;
}

void UZodiacGameplayAbility_Traversal::OnTraversalFinished()
{
	bHasEndedByNotify = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

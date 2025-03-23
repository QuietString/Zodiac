// the.quiet.string@gmail.com


#include "ZodiacHostAbility_Traversal.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacLogChannels.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Character/ZodiacCharacter.h"
#include "Traversal/ZodiacTraversalComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHostAbility_Traversal)

UZodiacHostAbility_Traversal::UZodiacHostAbility_Traversal(const FObjectInitializer& ObjectInitializer)
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

bool UZodiacHostAbility_Traversal::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
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
			FVector FrontLedgeNormal;
			AActor* BlockingActor = nullptr;
			bool Result = TraversalComponent->CanTraversalAction(FailReason, FrontLedgeNormal, BlockingActor);

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

void UZodiacHostAbility_Traversal::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (UZodiacTraversalComponent* TraversalComponent = ZodiacCharacter->FindComponentByClass<UZodiacTraversalComponent>())
		{
			TraversalComponent->OnTraversalFinished.BindUObject(this, &ThisClass::OnTraversalFinished);

			if (IsLocallyControlled())
			{
				TraversalComponent->PerformTraversalAction_Local();
			}

			// Automatically ends ability after some time in case.
			UAbilityTask_WaitDelay* WaitDelay =  UAbilityTask_WaitDelay::WaitDelay(this, 5.0f);
			WaitDelay->OnFinish.AddDynamic(this, &ThisClass::OnTraversalFinished);
			WaitDelay->Activate();
		}
	}
}

void UZodiacHostAbility_Traversal::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
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

void UZodiacHostAbility_Traversal::OnTraversalFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

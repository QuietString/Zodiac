// the.quiet.string@gmail.com


#include "ZodiacGameplayAbility_TraversalAction.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacLogChannels.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Character/ZodiacCharacter.h"
#include "Traversal/ZodiacTraversalComponent.h"
#include "Traversal/ZodiacTraversalTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacGameplayAbility_TraversalAction)

class UZodiacTraversalComponent;

UZodiacGameplayAbility_TraversalAction::UZodiacGameplayAbility_TraversalAction(const FObjectInitializer& ObjectInitializer)
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

bool UZodiacGameplayAbility_TraversalAction::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (UZodiacTraversalComponent* TraversalComponent = ZodiacCharacter->FindComponentByClass<UZodiacTraversalComponent>())
		{
			FText FailReason;
			bool Result = TraversalComponent->CanTraversalAction(FailReason);
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

void UZodiacGameplayAbility_TraversalAction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (UZodiacTraversalComponent* TraversalComponent = ZodiacCharacter->FindComponentByClass<UZodiacTraversalComponent>())
		{
			TraversalComponent->OnTraversalFinished.BindUObject(this, &ThisClass::OnTraversalFinished);
			if (HasAuthority(&CurrentActivationInfo))
			{
				TraversalComponent->PerformTraversalActionFromAbility();	
			}

			// Automatically ends ability after some time in case.
			UAbilityTask_WaitDelay* WaitDelay =  UAbilityTask_WaitDelay::WaitDelay(this, 5.0f);
			WaitDelay->OnFinish.AddDynamic(this, &ThisClass::OnTraversalFinished);
			WaitDelay->Activate();
		}
	}}

void UZodiacGameplayAbility_TraversalAction::OnTraversalFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

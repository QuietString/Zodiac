// the.quiet.string@gmail.com


#include "ZodiacGameplayAbility_Jump.h"

#include "ZodiacLogChannels.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Character/ZodiacCharacter.h"
#include "Traversal/ZodiacTraversalComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacGameplayAbility_Jump)

UZodiacGameplayAbility_Jump::UZodiacGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UZodiacGameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo();
	
	if (!ZodiacCharacter || !ZodiacCharacter->CanJump())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UZodiacGameplayAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (UZodiacTraversalComponent* TraversalComponent = ZodiacCharacter->FindComponentByClass<UZodiacTraversalComponent>())
		{
			FText FailReason;
			if (TraversalComponent->CanTraversalAction(FailReason))
			{
				TraversalComponent->TryActivateTraversalAbility();
			}
			else
			{
#if WITH_EDITOR
				if (ZodiacConsoleVariables::CVarTraversalDrawDebug.GetValueOnAnyThread())
				{
					UE_LOG(LogZodiacTraversal, Log, TEXT("Traversal Failed Reason: %s"), *FailReason.ToString());
				}
#endif
				CharacterJumpStart();
				
				UAbilityTask_WaitDelay* WaitDelay = UAbilityTask_WaitDelay::WaitDelay(this, DelayTime);
				WaitDelay->OnFinish.AddDynamic(this, &ThisClass::OnDelayFinished);
				WaitDelay->Activate();
				
				return;
			}
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UZodiacGameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop jumping in case the ability blueprint doesn't call it.
	CharacterJumpStop();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UZodiacGameplayAbility_Jump::OnDelayFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UZodiacGameplayAbility_Jump::CharacterJumpStart()
{
	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (ZodiacCharacter->IsLocallyControlled() && !ZodiacCharacter->bPressedJump)
		{
			ZodiacCharacter->UnCrouch();
			ZodiacCharacter->Jump();
		}
	}
}

void UZodiacGameplayAbility_Jump::CharacterJumpStop()
{
	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (ZodiacCharacter->IsLocallyControlled() && ZodiacCharacter->bPressedJump)
		{
			ZodiacCharacter->StopJumping();
		}
	}
}

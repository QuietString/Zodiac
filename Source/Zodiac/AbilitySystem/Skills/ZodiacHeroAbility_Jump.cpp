// the.quiet.string@gmail.com


#include "ZodiacHeroAbility_Jump.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacLogChannels.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/ZodiacCharacter.h"
#include "Traversal/ZodiacTraversalComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAbility_Jump)

UZodiacHeroAbility_Jump::UZodiacHeroAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UZodiacHeroAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

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

void UZodiacHeroAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilityTask_WaitGameplayEvent* WaitGameplayEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ZodiacGameplayTags::Event_JustLanded);
	WaitGameplayEvent->EventReceived.AddDynamic(this, &ThisClass::OnJustLanded);
	WaitGameplayEvent->Activate();
	
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
				bool Result = TraversalComponent->CanTraversalAction(FailReason);
#if WITH_EDITOR
				if (!Result && ZodiacConsoleVariables::CVarTraversalDrawDebug.GetValueOnAnyThread())
				{
					UE_LOG(LogZodiacTraversal, Log, TEXT("Traversal Failed Reason: %s"), *FailReason.ToString());
				}
#endif
				CharacterJumpStart();
				return;
			}
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UZodiacHeroAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop jumping in case the ability blueprint doesn't call it.
	CharacterJumpStop();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UZodiacHeroAbility_Jump::OnJustLanded(FGameplayEventData Payload)
{
	CharacterJumpStop();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UZodiacHeroAbility_Jump::CharacterJumpStart()
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

void UZodiacHeroAbility_Jump::CharacterJumpStop()
{
	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (ZodiacCharacter->IsLocallyControlled() && ZodiacCharacter->bPressedJump)
		{
			ZodiacCharacter->StopJumping();
		}
	}
}

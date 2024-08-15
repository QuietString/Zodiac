// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacSkillAbility_Jump.h"

#include "AbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "Character/ZodiacHostCharacter.h"
#include "Traversal/ZodiacTraversalComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacSkillAbility_Jump)

struct FGameplayTagContainer;


UZodiacSkillAbility_Jump::UZodiacSkillAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UZodiacSkillAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UAbilitySystemComponent* HeroASC = GetAbilitySystemComponentFromActorInfo())
	{
		UAbilityTask_WaitGameplayEvent* WaitGameplayEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, ZodiacGameplayTags::Event_JustLanded);
		WaitGameplayEvent->EventReceived.AddDynamic(this, &ThisClass::OnJustLanded);
		WaitGameplayEvent->Activate();
	}
	
	if (AZodiacHostCharacter* HostCharacter = GetZodiacHostCharacterFromActorInfo())
	{
		if (UZodiacCharacterMovementComponent* CharMovComp = Cast<UZodiacCharacterMovementComponent>(HostCharacter->GetCharacterMovement()))
		{
			if ((CharMovComp->CustomMovementMode != MOVE_Traversal) && CharMovComp->IsMovingOnGround())
			{
				bool bIsInAir = false;
				UZodiacTraversalComponent* TraversalComponent = HostCharacter->FindComponentByClass<UZodiacTraversalComponent>();
				if (!TraversalComponent->TryTraversalActionFromAbility(bIsInAir))
				{
					CharacterJumpStart();
					return;
				}
			}
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UZodiacSkillAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	const AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(ActorInfo->OwnerActor.Get());
	if (!HostCharacter || !HostCharacter->CanJump())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UZodiacSkillAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop jumping in case the ability blueprint doesn't call it.
	CharacterJumpStop();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UZodiacSkillAbility_Jump::CharacterJumpStart()
{
	if (AZodiacHostCharacter* HostCharacter = GetZodiacHostCharacterFromActorInfo())
	{
		if (HostCharacter->IsLocallyControlled() && !HostCharacter->bPressedJump)
		{
			HostCharacter->UnCrouch();
			HostCharacter->Jump();
		}
	}
}

void UZodiacSkillAbility_Jump::CharacterJumpStop()
{
	if (AZodiacHostCharacter* HostCharacter = GetZodiacHostCharacterFromActorInfo())
	{
		if (HostCharacter->IsLocallyControlled() && HostCharacter->bPressedJump)
		{
			HostCharacter->StopJumping();
		}
	}
}


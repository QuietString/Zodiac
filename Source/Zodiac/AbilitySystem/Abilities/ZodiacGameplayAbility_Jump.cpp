// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacGameplayAbility_Jump.h"
#include "Abilities/Tasks/AbilityTask_StartAbilityState.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "Character/ZodiacPlayerCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacGameplayAbility_Jump)

struct FGameplayTagContainer;


UZodiacGameplayAbility_Jump::UZodiacGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UZodiacGameplayAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CharacterJumpStart();

	UAbilityTask_StartAbilityState* JumpAbilityState = UAbilityTask_StartAbilityState::StartAbilityState(this, TEXT("Jumping"), true);
	//JumpAbilityState->OnStateEnded.AddDynamic(this, &ThisClass::UZodiacGameplayAbility_Jump::CharacterJumpStop);
	//JumpAbilityState->OnStateInterrupted.AddDynamic(this, &ThisClass::UZodiacGameplayAbility_Jump::CharacterJumpStop);
	JumpAbilityState->Activate();
	
	UAbilityTask_WaitInputRelease* WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	WaitInputRelease->OnRelease.AddDynamic(this, &ThisClass::OnInputRelease);
	WaitInputRelease->Activate();
}

bool UZodiacGameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	const AZodiacPlayerCharacter* PlayerCharacter = Cast<AZodiacPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!PlayerCharacter || !PlayerCharacter->CanJump())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UZodiacGameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop jumping in case the ability blueprint doesn't call it.
	CharacterJumpStop();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UZodiacGameplayAbility_Jump::OnInputRelease(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UZodiacGameplayAbility_Jump::CharacterJumpStart()
{
	if (AZodiacPlayerCharacter* PlayerCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (PlayerCharacter->IsLocallyControlled() && !PlayerCharacter->bPressedJump)
		{
			PlayerCharacter->UnCrouch();
			PlayerCharacter->Jump();
		}
	}
}

void UZodiacGameplayAbility_Jump::CharacterJumpStop()
{
	if (AZodiacPlayerCharacter* PlayerCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (PlayerCharacter->IsLocallyControlled() && PlayerCharacter->bPressedJump)
		{
			PlayerCharacter->StopJumping();
		}
	}
}


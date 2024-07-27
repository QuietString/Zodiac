// the.quiet.string@gmail.com


#include "ZodiacGameplayAbility_Sprint.h"

#include "EnhancedInputSubsystems.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Character/ZodiacHostCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UZodiacGameplayAbility_Sprint::UZodiacGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UZodiacGameplayAbility_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	WaitInputRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	WaitInputRelease->OnRelease.AddDynamic(this, &ThisClass::OnInputRelease);
	WaitInputRelease->Activate();
	
	Sprint();
}

void UZodiacGameplayAbility_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	WaitInputRelease->OnRelease.Clear();
	Walk();
}

UEnhancedInputLocalPlayerSubsystem* UZodiacGameplayAbility_Sprint::GetEnhancedInputSubsystem(AZodiacHostCharacter* ZodiacCharacter)
{
	if (const APlayerController* PC = ZodiacCharacter->GetController<APlayerController>())
	{
		if (const ULocalPlayer* LP = CastChecked<ULocalPlayer>(PC->GetLocalPlayer()))
		{
			return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		} 
	}

	return nullptr;
}

void UZodiacGameplayAbility_Sprint::Sprint()
{
	if (AZodiacHostCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* CharMoveComp =  ZodiacCharacter->GetCharacterMovement())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem(ZodiacCharacter))
			{
				FModifyContextOptions Options = {};
				Subsystem->AddMappingContext(SprintIMC, 1, Options);

				WalkingSpeed = CharMoveComp->MaxWalkSpeed;
				CharMoveComp->MaxWalkSpeed = SprintSpeed;
			}
		}
	}
}

void UZodiacGameplayAbility_Sprint::Walk()
{
	if (AZodiacHostCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* CharMoveComp =  ZodiacCharacter->GetCharacterMovement())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem(ZodiacCharacter))
			{
				FModifyContextOptions Options = {};
				Subsystem->RemoveMappingContext(SprintIMC, Options);
				
				CharMoveComp->MaxWalkSpeed = WalkingSpeed;
			}
		}
	}
}

void UZodiacGameplayAbility_Sprint::OnInputRelease(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

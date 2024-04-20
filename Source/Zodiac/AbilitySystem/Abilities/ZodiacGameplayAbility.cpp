// the.quiet.string@gmail.com


#include "ZodiacGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Character/ZodiacCharacter.h"
#include "ZodiacAbilitySimpleFailureMessage.h"
#include "GameFramework/GameplayMessageSubsystem.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_SIMPLE_FAILURE_MESSAGE, "Ability.UserFacingSimpleActivateFail.Message");
UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_PLAY_MONTAGE_FAILURE_MESSAGE, "Ability.PlayMontageOnActivateFail.Message");

UZodiacGameplayAbility::UZodiacGameplayAbility(const FObjectInitializer& ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationPolicy = EZodiacAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = EZodiacAbilityActivationGroup::Independent;
}

void UZodiacGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	UE_LOG(LogTemp, Warning, TEXT("ability given: %s"), *this->GetName());
}

void UZodiacGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// Testing damage to itself.
	// Apply a GameplayEffect to the owning actor to reduce health
	// if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	// {
	// 	AActor* OwningActor = ActorInfo->AvatarActor.Get();
	// 	if (OwningActor)
	// 	{
	// 		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	// 		if (ASC)
	// 		{
	// 			if (DamageEffect)
	// 			{
	// 				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	// 				EffectContext.AddSourceObject(this);
	//
	// 				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);
	// 				if (SpecHandle.IsValid())
	// 				{
	// 					FActiveGameplayEffectHandle GEHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	// 					UE_LOG(LogTemp, Warning, TEXT("ability applied"));
	// 				}	
	// 			}
	// 		}
	// 	}
	// }
	//
	// // End the ability immediately
	// EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

AZodiacCharacter* UZodiacGameplayAbility::GetZodiacCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AZodiacCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

void UZodiacGameplayAbility::NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
	bool bSimpleFailureFound = false;
	for (FGameplayTag Reason : FailedReason)
	{
		if (!bSimpleFailureFound)
		{
			if (const FText* pUserFacingMessage = FailureTagToUserFacingMessages.Find(Reason))
			{
				FZodiacAbilitySimpleFailureMessage Message;
				Message.PlayerController = GetActorInfo().PlayerController.Get();
				Message.FailureTags = FailedReason;
				Message.UserFacingReason = *pUserFacingMessage;

				UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
				MessageSystem.BroadcastMessage(TAG_ABILITY_SIMPLE_FAILURE_MESSAGE, Message);
				bSimpleFailureFound = true;
			}
		}
		
		if (UAnimMontage* pMontage = FailureTagToAnimMontage.FindRef(Reason))
		{
			FZodiacAbilityMontageFailureMessage Message;
			Message.PlayerController = GetActorInfo().PlayerController.Get();
			Message.FailureTags = FailedReason;
			Message.FailureMontage = pMontage;

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(TAG_ABILITY_PLAY_MONTAGE_FAILURE_MESSAGE, Message);
		}
	}
}
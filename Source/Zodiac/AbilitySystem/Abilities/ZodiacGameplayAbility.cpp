// the.quiet.string@gmail.com


#include "ZodiacGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Character/ZodiacCharacter.h"

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

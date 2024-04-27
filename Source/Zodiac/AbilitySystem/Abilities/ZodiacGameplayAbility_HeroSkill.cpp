// the.quiet.string@gmail.com


#include "AbilitySystem/Abilities/ZodiacGameplayAbility_HeroSkill.h"

#include "AbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"

void UZodiacGameplayAbility_HeroSkill::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                       const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	
	FGameplayEventData Payload;
	Payload.EventTag = ZodiacGameplayTags::GameplayEvent_Death;
	Payload.Target = ASC->GetAvatarActor();
	Payload.ContextHandle = ContextHandle;
	Payload.EventMagnitude = 1.0f;
	
	FScopedPredictionWindow NewScopedWindow(ASC, true);
	int32 Num = ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	UE_LOG(LogTemp, Warning, TEXT("activate character skill: %d "), Num);
}

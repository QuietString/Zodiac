// the.quiet.string@gmail.com


#include "ZodiacAbilitySystemBlueprintLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"

void UZodiacAbilitySystemBlueprintLibrary::SendGameplayEventToActorNotPredicted(AActor* Actor, FGameplayTag EventTag, FGameplayEventData Payload)
{
	if (::IsValid(Actor))
	{
		UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
		if (AbilitySystemComponent != nullptr && IsValidChecked(AbilitySystemComponent))
		{
			AbilitySystemComponent->HandleGameplayEvent(EventTag, &Payload);
		}
		else
		{
			ABILITY_LOG(Error, TEXT("UAbilitySystemBlueprintLibrary::SendGameplayEventToActor: Invalid ability system component retrieved from Actor %s. EventTag was %s"), *Actor->GetName(), *EventTag.ToString());
		}
	}
}

FGameplayEffectContextHandle UZodiacAbilitySystemBlueprintLibrary::AddInstigator(FGameplayEffectContextHandle ContextHandle, AActor* Instigator,
                                                                                 AActor* EffectCauser)
{
	FGameplayEffectContext* Context = ContextHandle.Get();
	if (Context)
	{
		Context->AddInstigator(Instigator, EffectCauser);
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UZodiacAbilitySystemBlueprintLibrary::AddInstigator called with invalid ContextHandle"));
	}

	return ContextHandle;
}
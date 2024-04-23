// the.quiet.string@gmail.com


#include "AbilitySystem/Abilities/ZodiacGameplayAbility_ChangeChar.h"

void UZodiacGameplayAbility_ChangeChar::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogTemp, Warning, TEXT("character change End ability and is cancled: %s"), bWasCancelled ? TEXT("true") : TEXT("false"));
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

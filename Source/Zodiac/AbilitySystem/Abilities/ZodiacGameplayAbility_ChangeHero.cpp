// the.quiet.string@gmail.com


#include "ZodiacGameplayAbility_ChangeHero.h"

#include "ZodiacGameplayTags.h"
#include "Character/ZodiacPlayerCharacter.h"

UZodiacGameplayAbility_ChangeHero::UZodiacGameplayAbility_ChangeHero(const FObjectInitializer& ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UZodiacGameplayAbility_ChangeHero::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Warning, TEXT("change hero: %d on NetMode: %d"), SlotIndex, GetAvatarActorFromActorInfo()->GetNetMode());

	if (SlotIndex != INDEX_NONE)
	{
		if (AZodiacPlayerCharacter* PlayerCharacter = Cast<AZodiacPlayerCharacter>(GetOwningActorFromActorInfo()))
		{
			PlayerCharacter->SetActiveHeroIndex(SlotIndex);
		}
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

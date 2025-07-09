// the.quiet.string@gmail.com


#include "ZodiacHostAbilitySystemComponent.h"

#include "AbilitySystem/Hero/ZodiacHeroAbilitySystemComponent.h"
#include "Character/Host/ZodiacHostCharacter.h"
#include "Character/Hero/ZodiacHeroActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHostAbilitySystemComponent)

void UZodiacHostAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags,
	const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, BlockTags, bExecuteCancelTags, CancelTags);

	AZodiacHostCharacter* HostCharacter = nullptr;
	if (AZodiacHostCharacter* FoundCharacter = Cast<AZodiacHostCharacter>(AbilityActorInfo->AvatarActor))
	{
		HostCharacter = FoundCharacter;
	}
	else if (AZodiacHostCharacter* FoundOwnerCharacter = Cast<AZodiacHostCharacter>(AbilityActorInfo->OwnerActor))
	{
		HostCharacter = FoundOwnerCharacter;
	}
	
	if (HostCharacter)
	{
		for (auto& Hero : HostCharacter->GetHeroes())
		{
			if (UZodiacHeroAbilitySystemComponent* HeroASC = Hero->GetHeroAbilitySystemComponent())
			{
				HeroASC->ApplyAbilityBlockAndCancelTags_FromHostASC(AbilityTags, nullptr, bEnableBlockTags, BlockTags, bExecuteCancelTags, CancelTags);
			}
		}
	}
}

void UZodiacHostAbilitySystemComponent::ApplyAbilityBlockAndCancelTags_FromHeroASC(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags,
	const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, BlockTags, bExecuteCancelTags, CancelTags);
}

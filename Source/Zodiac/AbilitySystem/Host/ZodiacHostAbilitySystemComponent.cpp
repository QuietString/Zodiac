// the.quiet.string@gmail.com


#include "ZodiacHostAbilitySystemComponent.h"

#include "Character/ZodiacHeroCharacter.h"
#include "Character/ZodiacHostCharacter.h"

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

	if (AZodiacHostCharacter* FoundCharacter = Cast<AZodiacHostCharacter>(AbilityActorInfo->OwnerActor))
	{
		HostCharacter = FoundCharacter;
	}
	
	if (HostCharacter)
	{
		for (auto& Hero : HostCharacter->GetHeroes())
		{
			if (UAbilitySystemComponent* ASC = Hero->GetAbilitySystemComponent())
			{
				ASC->ApplyAbilityBlockAndCancelTags(AbilityTags, nullptr, bEnableBlockTags, BlockTags, bExecuteCancelTags, CancelTags);
			}
		}
	}
}

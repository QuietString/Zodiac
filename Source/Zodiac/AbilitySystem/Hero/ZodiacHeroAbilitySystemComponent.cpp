// the.quiet.string@gmail.com


#include "ZodiacHeroAbilitySystemComponent.h"

#include "AbilitySystem/Host/ZodiacHostAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAbilitySystemComponent)

void UZodiacHeroAbilitySystemComponent::SetHostAbilitySystemComponent(UZodiacHostAbilitySystemComponent* InASC)
{
	check(InASC);
	HostAbilitySystemComponent = InASC;
}

void UZodiacHeroAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags,
	const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, BlockTags, bExecuteCancelTags, CancelTags);
	
	if (HostAbilitySystemComponent)
	{
		HostAbilitySystemComponent->ApplyAbilityBlockAndCancelTags_FromHeroASC(AbilityTags, nullptr, bEnableBlockTags, BlockTags, bExecuteCancelTags, CancelTags);
	}
}

void UZodiacHeroAbilitySystemComponent::ApplyAbilityBlockAndCancelTags_FromHostASC(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags,
	const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, BlockTags, bExecuteCancelTags, CancelTags);
}

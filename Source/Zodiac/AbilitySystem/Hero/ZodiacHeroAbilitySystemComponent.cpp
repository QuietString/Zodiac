// the.quiet.string@gmail.com


#include "ZodiacHeroAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAbilitySystemComponent)

void UZodiacHeroAbilitySystemComponent::SetHostAbilitySystemComponent(UZodiacAbilitySystemComponent* InASC)
{
	check(InASC);
	HostAbilitySystemComponent = InASC;
}

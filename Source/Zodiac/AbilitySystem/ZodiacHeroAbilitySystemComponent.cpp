// the.quiet.string@gmail.com


#include "ZodiacHeroAbilitySystemComponent.h"


void UZodiacHeroAbilitySystemComponent::SetHostAbilitySystemComponent(UZodiacAbilitySystemComponent* InASC)
{
	check(InASC);
	HostAbilitySystemComponent = InASC;
}

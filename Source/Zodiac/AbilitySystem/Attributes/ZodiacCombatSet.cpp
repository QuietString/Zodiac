// the.quiet.string@gmail.com


#include "ZodiacCombatSet.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacCombatSet)

UZodiacCombatSet::UZodiacCombatSet()
	: BaseDamage(50.0f)
	, BaseHeal(0.0f)
{
}

void UZodiacCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacCombatSet, BaseDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacCombatSet, BaseHeal, COND_None, REPNOTIFY_Always);
}

void UZodiacCombatSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZodiacCombatSet, BaseDamage, OldValue);
}

void UZodiacCombatSet::OnRep_BaseHeal(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZodiacCombatSet, BaseHeal, OldValue);
}

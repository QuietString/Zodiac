// the.quiet.string@gmail.com


#include "ZodiacCombatSet.h"

#include "Net/UnrealNetwork.h"

UZodiacCombatSet::UZodiacCombatSet()
	: Mana(50.0f)
{
}

void UZodiacCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacCombatSet, Mana, COND_None, REPNOTIFY_Always);
}

void UZodiacCombatSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZodiacCombatSet, Mana, OldValue);
}

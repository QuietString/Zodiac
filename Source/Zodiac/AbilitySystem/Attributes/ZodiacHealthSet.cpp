// the.quiet.string@gmail.com


#include "AbilitySystem/Attributes/ZodiacHealthSet.h"

#include "Net/UnrealNetwork.h"

UZodiacHealthSet::UZodiacHealthSet()
	: Health(100.0f)
{
}

void UZodiacHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacHealthSet, Health, COND_None, REPNOTIFY_Always);
}

void UZodiacHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZodiacHealthSet, Health, OldValue)
}

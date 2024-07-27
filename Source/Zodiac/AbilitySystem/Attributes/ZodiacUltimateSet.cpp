// the.quiet.string@gmail.com


#include "AbilitySystem/Attributes/ZodiacUltimateSet.h"

#include "Net/UnrealNetwork.h"

UZodiacUltimateSet::UZodiacUltimateSet()
	: UltimateGauge(0.0f)
	, MaxUltimateGauge(100.0f)
{
}

void UZodiacUltimateSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacUltimateSet, UltimateGauge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacUltimateSet, MaxUltimateGauge, COND_None, REPNOTIFY_Always);
}

void UZodiacUltimateSet::OnRep_UltimateGauge(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZodiacUltimateSet, UltimateGauge, OldValue);
}
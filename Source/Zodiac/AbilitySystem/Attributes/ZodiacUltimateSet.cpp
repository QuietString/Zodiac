// the.quiet.string@gmail.com


#include "AbilitySystem/Attributes/ZodiacUltimateSet.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacUltimateSet)

UZodiacUltimateSet::UZodiacUltimateSet()
	: Ultimate(0.0f)
	, MaxUltimate(100.0f)
{
}

void UZodiacUltimateSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacUltimateSet, Ultimate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacUltimateSet, MaxUltimate, COND_None, REPNOTIFY_Always);
}

void UZodiacUltimateSet::OnRep_Ultimate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZodiacUltimateSet, Ultimate, OldValue);
}
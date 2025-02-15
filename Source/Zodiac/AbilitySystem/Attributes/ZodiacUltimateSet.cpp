// the.quiet.string@gmail.com


#include "AbilitySystem/Attributes/ZodiacUltimateSet.h"

#include "GameplayEffectExtension.h"
#include "ZodiacGameplayTags.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacUltimateSet)

UZodiacUltimateSet::UZodiacUltimateSet()
	: Ultimate(0.0f)
	, MaxUltimate(100.0f)
	, InitialCheckAmount(0.f)
	, InitialApplyAmount(0.f)
{
}

void UZodiacUltimateSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacUltimateSet, Ultimate, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacUltimateSet, MaxUltimate, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacUltimateSet, InitialCheckAmount, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacUltimateSet, InitialApplyAmount, COND_None, REPNOTIFY_OnChanged);
}

void UZodiacUltimateSet::OnRep_Ultimate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZodiacUltimateSet, Ultimate, OldValue);
}

void UZodiacUltimateSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UZodiacUltimateSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UZodiacUltimateSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	ClampAttribute(Attribute, NewValue);
}

bool UZodiacUltimateSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	if (Data.EvaluatedData.Attribute == GetUltimateAttribute())
	{
		if (Data.EvaluatedData.Magnitude < 0.f)
		{
			if (Data.Target.HasMatchingGameplayTag(ZodiacGameplayTags::Cheat_InfiniteUltimate))
			{
				Data.EvaluatedData.Magnitude = 0.f;
				return false;
			}
		}
	}
	
	return Super::PreGameplayEffectExecute(Data);
}

void UZodiacUltimateSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	float MinimumUltimate = 0.f;
	
	if (Data.EvaluatedData.Attribute == GetUltimateAttribute())
	{
		SetUltimate(FMath::Clamp(GetUltimate(), MinimumUltimate , GetMaxUltimate()));
	}
}

void UZodiacUltimateSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetUltimateAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxUltimate());
	}
	else if (Attribute == GetMaxUltimateAttribute())
	{
		// Do not allow max health to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

// the.quiet.string@gmail.com


#include "AbilitySystem/Attributes/ZodiacHealthSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

namespace ZodiacConsoleVariables
{
	static bool EnableLogHealthChange = false;
	static FAutoConsoleVariableRef CVarEnableLogHealthChange(
		TEXT("zodiac.LogChannel.HealthChange"),
		EnableLogHealthChange,
		TEXT("Should we log debug information of health change"),
		ECVF_Default);
}

UZodiacHealthSet::UZodiacHealthSet()
	: Health(100.0f)
	, MaxHealth(100.0f)
{
	bOutOfHealth = false;
	MaxHealthBeforeAttributeChange = 0.0f;
	HealthBeforeAttributeChange = 0.0f;
}

void UZodiacHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UZodiacHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UZodiacHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZodiacHealthSet, Health, OldValue)

	const float CurrentHealth = GetHealth();
	const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();

	if (!bOutOfHealth && CurrentHealth <= 0.0f)
	{
		OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);
	}

#if WITH_EDITOR
	if (ZodiacConsoleVariables::EnableLogHealthChange)
	{
		UE_LOG(LogTemp, Warning, TEXT("health: %.1f"), CurrentHealth);
	}
#endif
	bOutOfHealth = (CurrentHealth <= 0.0f);
}

void UZodiacHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UZodiacHealthSet, MaxHealth, OldValue);
	
	OnMaxHealthChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxHealth() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxHealth());
}

bool UZodiacHealthSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	// if (!Super::PreGameplayEffectExecute(Data))
	// {
	// 	return false;
	// }
	//
	// // Save the current health
	// HealthBeforeAttributeChange = GetHealth();
	// MaxHealthBeforeAttributeChange = GetMaxHealth();
	
	return Super::PreGameplayEffectExecute(Data);
}

void UZodiacHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	float MinimumHealth = 0.0f;

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();
	
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));
		SetDamage(0.0f);
	}
	
	// else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	// {
	// 	// Clamp and fall into out of health handling below
	// 	SetHealth(FMath::Clamp(GetHealth(), MinimumHealth, GetMaxHealth()));
	// }

	if (GetHealth() <= 0.0f && !bOutOfHealth)
	{
		OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, 0, GetHealth());	
	}
}

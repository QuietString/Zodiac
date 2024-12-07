// the.quiet.string@gmail.com

#include "ZodiacHeroAttributeSet_Twinblast.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAttributeSet_Twinblast)

UZodiacHeroAttributeSet_Twinblast::UZodiacHeroAttributeSet_Twinblast()
	: Fuel(100.f)
	, MaxFuel(100.f)
{
	bOutOfFuel = false;
	MaxFuelBeforeAttributeChange = 0.f;
	FuelBeforeAttributeChange = 0.f;
}

void UZodiacHeroAttributeSet_Twinblast::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Fuel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxFuel, COND_None, REPNOTIFY_Always);
}

void UZodiacHeroAttributeSet_Twinblast::OnRep_Fuel(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Fuel, OldValue)

	const float CurrentFuel = GetFuel();
	const float EstimatedMagnitude = CurrentFuel - OldValue.GetCurrentValue();

	OnFuelChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentFuel);

	if (!bOutOfFuel && CurrentFuel <= 0.0f)
	{
		OnOutOfFuel.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentFuel);
	}

	bOutOfFuel = (CurrentFuel <= 0.0f);
}

void UZodiacHeroAttributeSet_Twinblast::OnRep_MaxFuel(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, MaxFuel, OldValue);

	OnMaxFuelChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxFuel() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxFuel());
}

bool UZodiacHeroAttributeSet_Twinblast::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}
	
	// Save the current fuel
	FuelBeforeAttributeChange = GetFuel();
	MaxFuelBeforeAttributeChange = GetMaxFuel();
	
	return Super::PreGameplayEffectExecute(Data);
}

void UZodiacHeroAttributeSet_Twinblast::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	float MinimumFuel = 0.0f;
	
	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();
	
	if (Data.EvaluatedData.Attribute == GetFuelConsumptionAttribute())
	{
		SetFuel(FMath::Clamp(GetFuel() - GetFuelConsumption(), MinimumFuel, GetMaxFuel()));
		SetFuelConsumption(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetFuelChargeAttribute())
	{
		SetFuel(FMath::Clamp(GetFuel() + GetFuelCharge(), MinimumFuel, GetMaxFuel()));
		SetFuelCharge(0.f);
	}
	else if (Data.EvaluatedData.Attribute == GetFuelAttribute())
	{
		// Clamp and fall into out of fuel handling below
		SetFuel(FMath::Clamp(GetFuel(), MinimumFuel, GetMaxFuel()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxFuelAttribute())
	{
		// Notify on any requested max fuel changes
		OnMaxFuelChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxFuelBeforeAttributeChange, GetMaxFuel());
	}

	// If fuel has actually changed activate callbacks
	if (GetFuel() != FuelBeforeAttributeChange)
	{
		OnFuelChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, FuelBeforeAttributeChange, GetFuel());
	}

	if ((GetFuel() <= 0.0f) && !bOutOfFuel)
	{
		OnOutOfFuel.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, FuelBeforeAttributeChange, GetFuel());
	}

	// Check fuel again in case an event above changed it.
	bOutOfFuel = (GetFuel() <= 0.0f);
}

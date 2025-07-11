// the.quiet.string@gmail.com

#include "Character/ZodiacHealthComponent.h"

#include "ZodiacCharacter.h"
#include "ZodiacLogChannels.h"
#include "ZodiacGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Hero/ZodiacHeroActor.h"
#include "Messages/ZodiacMessageTypes.h"
#include "Messages/ZodiacVerbMessageHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHealthComponent)

namespace ZodiacConsoleVariables
{
	static bool EnableLogHealthChange = false;
	static FAutoConsoleVariableRef CVarEnableLogHealthChange(
		TEXT("zodiac.LogChannel.HealthChange"),
		EnableLogHealthChange,
		TEXT("Should we log debug information of health change"),
		ECVF_Default);
}

UZodiacHealthComponent::UZodiacHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
	HealthSet = nullptr;
	DeathState = EZodiacDeathState::NotDead;
}

UZodiacHealthComponent* UZodiacHealthComponent::FindHealthComponent(const AActor* Actor)
{
	{ return (Actor ? Actor->FindComponentByClass<UZodiacHealthComponent>() : nullptr); }
}

void UZodiacHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UZodiacHealthComponent, DeathState);
}

void UZodiacHealthComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}

void UZodiacHealthComponent::InitializeWithAbilitySystem(UZodiacAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogZodiac, Error, TEXT("ZodiacHealthComponent: Health component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogZodiac, Error, TEXT("ZodiacHealthComponent: Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	HealthSet = AbilitySystemComponent->GetSet<UZodiacHealthSet>();
	if (!HealthSet)
	{
		UE_LOG(LogZodiac, Error, TEXT("ZodiacHealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	// Register to listen for attribute changes.
	HealthSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
	HealthSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);

	// TEMP: Reset attributes to default values.  Eventually this will be driven by a spreadsheet.
	AbilitySystemComponent->SetNumericAttributeBase(UZodiacHealthSet::GetHealthAttribute(), HealthSet->GetMaxHealth());

	ClearGameplayTags();

	OnHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
	OnMaxHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
}

void UZodiacHealthComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();

	if (HealthSet)
	{
		HealthSet->OnHealthChanged.RemoveAll(this);
		HealthSet->OnMaxHealthChanged.RemoveAll(this);
		HealthSet->OnOutOfHealth.RemoveAll(this);
	}

	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void UZodiacHealthComponent::ClearGameplayTags()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(ZodiacGameplayTags::Status_Death_Dying, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(ZodiacGameplayTags::Status_Death_Dead, 0);
	}
}

float UZodiacHealthComponent::GetHealth() const
{
	return (HealthSet ? HealthSet->GetHealth() : 0.0f);
}

float UZodiacHealthComponent::GetMaxHealth() const
{
	return (HealthSet ? HealthSet->GetMaxHealth() : 0.0f);
}

float UZodiacHealthComponent::GetHealthNormalized() const
{
	if (HealthSet)
	{
		const float Health = HealthSet->GetHealth();
		const float MaxHealth = HealthSet->GetMaxHealth();

		return ((MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f);
	}

	return 0.0f;
}

void UZodiacHealthComponent::HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);

#if WITH_EDITOR
	if (ZodiacConsoleVariables::EnableLogHealthChange)
	{
		UE_LOG(LogZodiac, Warning, TEXT("%s's health changed from %.1f to %.1f"), *GetOwner()->GetName(), OldValue, NewValue);
	}
#endif
}

void UZodiacHealthComponent::HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UZodiacHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	// Instigator: HostCharacter
	// Causer: HeroCharacter
	// Source: AbilitySlot
	
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && DamageEffectSpec)
	{
		// Send the "Event.Death" gameplay event through the owner's ability system.  This can be used to trigger a death gameplay ability.
		{
			FGameplayEventData Payload;
			Payload.EventTag = ZodiacGameplayTags::Event_Death;
			Payload.Instigator = DamageInstigator;
			Payload.Target = AbilitySystemComponent->GetAvatarActor();
			Payload.OptionalObject = DamageEffectSpec->Def;
			Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
			Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = DamageMagnitude;

			FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
			AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
		}

		// Send the "Event.Elimination" gameplay event to the instigator's ability system.  This can be used to trigger an OnElimination gameplay ability.
		{
			if (AZodiacHeroActor* HeroActor = Cast<AZodiacHeroActor>(DamageCauser))
			{
				if (UAbilitySystemComponent* InstigatorASC = HeroActor->GetAbilitySystemComponent())
				{
					FGameplayEventData Payload;
					Payload.EventTag = ZodiacGameplayTags::Event_Elimination;
					Payload.Instigator = DamageInstigator;
					Payload.Target = AbilitySystemComponent->GetAvatarActor();
					Payload.OptionalObject = DamageEffectSpec->Def;
					Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
					Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
					Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
					Payload.EventMagnitude = DamageMagnitude;
		
					InstigatorASC->HandleGameplayEvent(Payload.EventTag, &Payload);
				}	
			}
		}
		
		// Send a standardized verb message that other systems can observe. For cosmetic usages like elimination marker.
		{
			FZodiacVerbMessage Message;
			Message.Channel = ZodiacGameplayTags::Message_Gameplay_Elimination;
			Message.Instigator = DamageInstigator;
			Message.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Message.Target = AbilitySystemComponent->GetAvatarActor();
			Message.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(Message.Channel, Message);
		}
	}
#endif
}

void UZodiacHealthComponent::OnRep_DeathState(EZodiacDeathState OldDeathState)
{
	const EZodiacDeathState NewDeathState = DeathState;

	// Revert the death state for now since we rely on StartDeath and FinishDeath to change it.
	DeathState = OldDeathState;

	if (OldDeathState > NewDeathState)
	{
		// The server is trying to set us back, but we've already predicted past the server state.
		UE_LOG(LogZodiac, Warning, TEXT("ZodiacHealthComponent: Predicted past server death state [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		return;
	}

	if (OldDeathState == EZodiacDeathState::NotDead)
	{
		if (NewDeathState == EZodiacDeathState::DeathStarted)
		{
			StartDeath();
		}
		else if (NewDeathState == EZodiacDeathState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
		else
		{
			UE_LOG(LogZodiac, Error, TEXT("ZodiacHealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
	else if (OldDeathState == EZodiacDeathState::DeathStarted)
	{
		if (NewDeathState == EZodiacDeathState::DeathFinished)
		{
			FinishDeath();
		}
		else
		{
			UE_LOG(LogZodiac, Error, TEXT("ZodiacHealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}

	ensureMsgf((DeathState == NewDeathState), TEXT("ZodiacHealthComponent: Death transition failed [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
}

void UZodiacHealthComponent::StartDeath()
{
	if (DeathState != EZodiacDeathState::NotDead)
	{
		return;
	}

	DeathState = EZodiacDeathState::DeathStarted;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(ZodiacGameplayTags::Status_Death_Dying, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathStarted.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void UZodiacHealthComponent::FinishDeath()
{
	if (DeathState != EZodiacDeathState::DeathStarted)
	{
		return;
	}

	DeathState = EZodiacDeathState::DeathFinished;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(ZodiacGameplayTags::Status_Death_Dead, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathFinished.Broadcast(Owner);
	
	Owner->ForceNetUpdate();
}

void UZodiacHealthComponent::ResetHealthAndDeathState()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetNumericAttributeBase(UZodiacHealthSet::GetHealthAttribute(), HealthSet->GetMaxHealth());
	}

	DeathState = EZodiacDeathState::NotDead;
	ClearGameplayTags();
}

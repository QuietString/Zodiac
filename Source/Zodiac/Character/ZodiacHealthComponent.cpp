// the.quiet.string@gmail.com


#include "Character/ZodiacHealthComponent.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"
#include "Net/UnrealNetwork.h"

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

void UZodiacHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UZodiacHealthComponent, DeathState);
}

void UZodiacHealthComponent::InitializeWithAbilitySystem(const int32 InSlotIndex, UZodiacAbilitySystemComponent* InASC)
{
	check(GetOwner());

	SlotIndex = InSlotIndex;
	AbilitySystemComponent = InASC;
	
	HealthSet = CastChecked<UZodiacHealthSet>(InASC->GetAttributeSet(UZodiacHealthSet::StaticClass()));
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);
	
	ClearGameplayTags();
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
	
}

void UZodiacHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser,
                                               const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && DamageEffectSpec)
	{
		// Send the "GameplayEvent.Death" gameplay event through the owner's ability system.  This can be used to trigger a death gameplay ability.
		{
			FGameplayEventData Payload;
			Payload.EventTag = ZodiacGameplayTags::GameplayEvent_Death;
			Payload.Instigator = DamageInstigator;
			Payload.Target = AbilitySystemComponent->GetAvatarActor();
			Payload.OptionalObject = DamageEffectSpec->Def;
			Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
			Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = DamageMagnitude;
			
			FScopedPredictionWindow NewScopedWindow(AbilitySystemComponent, true);
			int32 Num = AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
		}
		
		// Send a standardized verb message that other systems can observe
		// {
		// 	FZodiacVerbMessage Message;
		// 	Message.Verb = TAG_Zodiac_Elimination_Message;
		// 	Message.Instigator = DamageInstigator;
		// 	Message.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
		// 	Message.Target = UZodiacVerbMessageHelpers::GetPlayerStateFromObject(AbilitySystemComponent->GetAvatarActor());
		// 	Message.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
		// 	//@TODO: Fill out context tags, and any non-ability-system source/instigator tags
		// 	//@TODO: Determine if it's an opposing team kill, self-own, team kill, etc...
		//
		// 	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		// 	MessageSystem.BroadcastMessage(Message.Verb, Message);
		// }

		//@TODO: assist messages (could compute from damage dealt elsewhere)?
	}

#endif
}

void UZodiacHealthComponent::ClearGameplayTags()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(ZodiacGameplayTags::Status_Death_Dying, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(ZodiacGameplayTags::Status_Death_Dead, 0);
	}
}

void UZodiacHealthComponent::OnRep_DeathState(EZodiacDeathState OldDeathState)
{
	const EZodiacDeathState NewDeathState = DeathState;

	// Revert the death state for now since we rely on StartDeath and FinishDeath to change it.
	DeathState = OldDeathState;

	if (OldDeathState > NewDeathState)
	{
		// The server is trying to set us back but we've already predicted past the server state.
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
// the.quiet.string@gmail.com


#include "ZodiacSkillManagerComponent.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacHeroComponent.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/ZodiacAbilityCost.h"
#include "AbilitySystem/Abilities/ZodiacSkillAbilityCost_TagStack.h"
#include "AbilitySystem/Abilities/ZodiacSkillCost_TagStack.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"
#include "AbilitySystem/Attributes/ZodiacUltimateSet.h"
#include "AbilitySystem/Skills/ZodiacSkillSet.h"
#include "AbilitySystem/Skills/ZodiacSkillSlot.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/ZodiacMessageLibrary.h"
#include "Messages/ZodiacMessageTypes.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacSkillManagerComponent)

UZodiacSkillManagerComponent::UZodiacSkillManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bReplicateUsingRegisteredSubObjectList = true;
	SetIsReplicatedByDefault(true);
	AbilitySystemComponent = nullptr;
	HealthSet = nullptr;
	UltimateSet = nullptr;
}

void UZodiacSkillManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Slots);
}

void UZodiacSkillManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	
	// Register existing ZodiacSkillSlot
	if (IsUsingRegisteredSubObjectList())
	{
		for (const auto& Slot : Slots)
		{
			if (Slot)
			{
				AddReplicatedSubObject(Slot);
			}
		}
	}
}

void UZodiacSkillManagerComponent::BeginPlay()
{
	if (HasAuthority())
	{
		for (UZodiacSkillSlot* Slot : Slots)
		{
			for (auto& [Key, Value] : Slot->GetSlotDefinition()->InitialTagStack)
			{
				Slot->AddStatTagStack(Key, Value);
			}
		}	
	}
	
	SendSlotWidgetCreatedMessage();
	
	if (HeroIndex == 0)
	{
		SendResetMessages();	
	}
	
	Super::BeginPlay();
}

void UZodiacSkillManagerComponent::InitializeSlots(UZodiacHeroComponent* HeroComponent,
                                                   TMap<FGameplayTag, TObjectPtr<UZodiacSkillSlotDefinition>> SlotDefinitions)
{
	check(HeroComponent);
	UZodiacAbilitySystemComponent* ZodiacASC = HeroComponent->GetZodiacAbilitySystemComponent();
	check(ZodiacASC);
	
	HeroIndex = HeroComponent->GetSlotIndex();
	AbilitySystemComponent = ZodiacASC;
	
	if (HasAuthority())
	{
		for (auto& [SlotType, Definition] : SlotDefinitions)
		{
			UZodiacSkillSlot* Slot = Slots.Add_GetRef(NewObject<UZodiacSkillSlot>(GetOwner(), UZodiacSkillSlot::StaticClass()));
			Slot->InitializeSlot(Definition, SlotType);
			Slot->CreateSlotWidget();
			
			if (TObjectPtr<UZodiacSkillSet> SkillSet = Definition->SkillSetToGrant)
			{
				Definition->SkillSetToGrant->GiveToAbilitySystem(ZodiacASC, &Slot->GrantedHandles, Slot);
			}
			
			if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Slot)
			{
				AddReplicatedSubObject(Slot);
			}
		}	
	}
	
	// for displaying UI elements
	HeroComponent->OnHeroChanged_Simple.AddUObject(this, &ThisClass::OnHeroChanged);
	
	HealthSet = CastChecked<UZodiacHealthSet>(AbilitySystemComponent->GetAttributeSet(UZodiacHealthSet::StaticClass()));
	//CombatSet = CastChecked<UZodiacCombatSet>(InASC->GetAttributeSet(UZodiacCombatSet::StaticClass()));
	UltimateSet = CastChecked<UZodiacUltimateSet>(AbilitySystemComponent->GetAttributeSet(UZodiacUltimateSet::StaticClass()));
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UZodiacHealthSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UZodiacUltimateSet::GetUltimateGaugeAttribute()).AddUObject(this, &ThisClass::HandleUltimateGaugeChanged);
}

void UZodiacSkillManagerComponent::OnHeroChanged()
{
	SendHealthBarHeroChangedMessage();
	//SendSkillSlotChangedMessages();
	SendSkillSlotChangedMessages();
}

void UZodiacSkillManagerComponent::HandleHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	float NewValue = OnAttributeChangeData.NewValue;
	float OldValue = OnAttributeChangeData.OldValue;

	FHeroValueChangedMessage Message;
	Message.Instigator = GetPawn<APawn>();
	Message.SlotIndex = HeroIndex;
	Message.OldValue = OldValue;
	Message.NewValue = NewValue;
	Message.MaxValue = HealthSet->GetMaxHealth();
	
	const FGameplayTag Channel = ZodiacGameplayTags::GameplayEvent_Damaged_Message;
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(Channel, Message);
}

void UZodiacSkillManagerComponent::HandleUltimateGaugeChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	{
		float NewValue = OnAttributeChangeData.NewValue;
		float OldValue = OnAttributeChangeData.OldValue;

		FHeroValueChangedMessage Message;
		Message.Instigator = GetPawn<APawn>();
		Message.SlotIndex = HeroIndex;
		Message.OldValue = OldValue;
		Message.NewValue = NewValue;
		Message.OptionalValue = RequiredUltimateCostAmount;
	
		const FGameplayTag Channel = UZodiacMessageLibrary::GetUltimateChargeChannel();
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSubsystem.BroadcastMessage(Channel, Message);	
	}

	{
		float NewValue = OnAttributeChangeData.NewValue;
		float OldValue = OnAttributeChangeData.OldValue;

		FZodiacHUDMessage_AttributeChanged Message;
		Message.Instigator = GetPawn<APawn>();
		Message.HeroIndex = HeroIndex;
		Message.OldValue = OldValue;
		Message.NewValue = NewValue;
		
		const FGameplayTag Channel = ZodiacGameplayTags::HUD_Message_AttributeChanged_Ultimate;
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSubsystem.BroadcastMessage(Channel, Message);	
	}
}

void UZodiacSkillManagerComponent::SendResetMessages()
{
	//SendSkillSlotChangedMessages();
	SendSkillSlotChangedMessages();
	
	FOnAttributeChangeData Data;
	Data.NewValue = HealthSet->GetHealth();
	Data.OldValue = HealthSet->GetHealth();
	HandleHealthChanged(Data);
}

void UZodiacSkillManagerComponent::SendSlotWidgetCreatedMessage()
{
	for (UZodiacSkillSlot* Slot : Slots)
	{
		if (Slot->GetSlotWidget())
		{
			FZodiacHUDMessage_SlotCreated Message;
			Message.Instigator = GetPawn<APawn>();
			Message.Widget = Slot->GetSlotWidget();
			Message.HeroIndex = HeroIndex;
			Message.Slot = Slot->GetSlotType();
				
			const FGameplayTag Channel = ZodiacGameplayTags::HUD_Message_SkillSlot_Created;
			UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSubsystem.BroadcastMessage(Channel, Message);	
		}
	}
}

void UZodiacSkillManagerComponent::SendHealthBarHeroChangedMessage()
{
	float CurrentHealth = HealthSet->GetHealth();
	float MaxHealth = HealthSet->GetMaxHealth();
	
	//UE_LOG(LogTemp, Warning, TEXT("current health: %.1f, max health: %.1f"), CurrentHealth, MaxHealth);
	FHeroValueChangedMessage Message;
	Message.SlotIndex = HeroIndex;
	Message.Instigator = GetPawn<APawn>();
	Message.NewValue = CurrentHealth;
	Message.MaxValue = MaxHealth;

	const FGameplayTag Channel = ZodiacGameplayTags::Hero_Changed_HealthBar_Message;
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(Channel, Message);
}

void UZodiacSkillManagerComponent::SendSkillSlotChangedMessages()
{
	FZodiacHUDMessage_HeroChanged Message;
	Message.Instigator = GetPawn<APawn>();
	Message.HeroIndex = HeroIndex;
	
	const FGameplayTag Channel = ZodiacGameplayTags::HUD_Message_HeroChanged;
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(Channel, Message);
}

void UZodiacSkillManagerComponent::GetUltimateGauge(FHeroChangedMessage_SkillSlot& OutMessage)
{
	OutMessage.CurrentValue = UltimateSet->GetUltimateGauge();
	OutMessage.MaxValue = UltimateSet->GetMaxUltimateGauge();
	OutMessage.OptionalValue = RequiredUltimateCostAmount;
}

void UZodiacSkillManagerComponent::GetCooldown(FHeroChangedMessage_SkillSlot& OutMessage, FGameplayTag SlotType)
{
	FGameplayTagContainer QueryContainer;
	QueryContainer.AddTag(SlotType);
	FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(QueryContainer);
	TArray< TPair<float,float> > DurationAndTimeRemaining = AbilitySystemComponent->GetActiveEffectsTimeRemainingAndDuration(Query);
	if (DurationAndTimeRemaining.Num() > 0)
	{
		int32 BestIdx = 0;
		float LongestTime = DurationAndTimeRemaining[0].Key;
		for (int32 Idx = 1; Idx < DurationAndTimeRemaining.Num(); ++Idx)
		{
			if (DurationAndTimeRemaining[Idx].Key > LongestTime)
			{
				LongestTime = DurationAndTimeRemaining[Idx].Key;
				BestIdx = Idx;
			}
		}
		OutMessage.Cooldown_Remaining = DurationAndTimeRemaining[BestIdx].Key;
		OutMessage.Cooldown_Duration = DurationAndTimeRemaining[BestIdx].Value;
		OutMessage.bIsReady = false;
		UE_LOG(LogTemp, Warning, TEXT("cool down: %.1f, remaining: %.1f"), OutMessage.Cooldown_Duration, OutMessage.Cooldown_Remaining );
	}
	else
	{
		OutMessage.Cooldown_Remaining = 0;
		OutMessage.Cooldown_Duration = BIG_NUMBER;
		OutMessage.bIsReady = true;
	}
}
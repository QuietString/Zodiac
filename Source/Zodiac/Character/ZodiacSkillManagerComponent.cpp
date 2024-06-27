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
#include "AbilitySystem/Skills/ZodiacSkillAbility.h"
#include "AbilitySystem/Skills/ZodiacSkillSet.h"
#include "AbilitySystem/Skills/ZodiacSkillSlot.h"
#include "AbilitySystem/Skills/ZodiacSkillSlotFragment_SlotIcon.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/ZodiacMessageLibrary.h"
#include "Messages/ZodiacMessageTypes.h"
#include "Net/UnrealNetwork.h"

FString FZodiacSkillSlotEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s of %s"), *GetNameSafe(Slot), *GetNameSafe(SlotDefinition.Get()));
}

UZodiacSkillSlot* FZodiacSkillSlotList::AddEntry(TObjectPtr<UZodiacSkillSlotDefinition> SlotDefinition, FGameplayTag
                                                 SlotType, TObjectPtr<UZodiacAbilitySystemComponent> ZodiacASC)
{
	UZodiacSkillSlot* Result = nullptr;

	check(ZodiacASC);
	check(SlotDefinition);
	check(OwnerComponent);
	check(OwnerComponent->GetOwner()->HasAuthority());

	FZodiacSkillSlotEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.SlotDefinition = SlotDefinition;
	NewEntry.Slot = NewObject<UZodiacSkillSlot>(OwnerComponent->GetOwner(), UZodiacSkillSlot::StaticClass());
	
	Result = NewEntry.Slot;
	Result->SetSlotType(SlotType);
	Result->SetSlotDefinition(SlotDefinition);
	for (auto& [Key, Value] : SlotDefinition->InitialTagStack)
	{
		Result->AddStatTagStack(Key, Value);
	}
	
	if (TObjectPtr<UZodiacSkillSet> SkillSet = SlotDefinition->SkillSetToGrant)
	{
		SlotDefinition->SkillSetToGrant->GiveToAbilitySystem(ZodiacASC, IN OUT &NewEntry.GrantedHandles, Result);
	}

	MarkItemDirty(NewEntry);

	return Result;
}

UZodiacSkillManagerComponent::UZodiacSkillManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SkillSlotList(this)
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

	DOREPLIFETIME(ThisClass, SkillSlotList);
}

bool UZodiacSkillManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	//UE_LOG(LogTemp, Warning, TEXT("replicate subobjects"));
	for (FZodiacSkillSlotEntry& Entry : SkillSlotList.Entries)
	{
		UZodiacSkillSlot* Instance = Entry.Slot;

		if (IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

void UZodiacSkillManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing ZodiacSkillSlot
	if (IsUsingRegisteredSubObjectList())
	{
		//UE_LOG(LogTemp, Warning, TEXT("use subobject list"));
		for (const FZodiacSkillSlotEntry& Entry : SkillSlotList.Entries)
		{
			UZodiacSkillSlot* Instance = Entry.Slot;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}

	if (SlotIndex == 0)
	{
		SendResetMessages();
	}
}

void UZodiacSkillManagerComponent::InitializeSlots(UZodiacHeroComponent* HeroComponent,
                                                   TMap<FGameplayTag, TObjectPtr<UZodiacSkillSlotDefinition>> SlotDefinitions)
{
	check(HeroComponent);
	UZodiacAbilitySystemComponent* ZodiacASC = HeroComponent->GetZodiacAbilitySystemComponent();
	check(ZodiacASC);
	
	SlotIndex = HeroComponent->GetSlotIndex();
	AbilitySystemComponent = ZodiacASC;

	if (HasAuthority())
	{
		for (auto& [SlotType, Definition] : SlotDefinitions)
		{
			UZodiacSkillSlot* Slot = SkillSlotList.AddEntry(Definition, SlotType, ZodiacASC);
			Slot->OnTagStackChanged.BindUObject(this, &ThisClass::SendSlotStatTagChangedMessage);
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
	SendSkillSlotChangedMessages();
}

void UZodiacSkillManagerComponent::HandleHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	float NewValue = OnAttributeChangeData.NewValue;
	float OldValue = OnAttributeChangeData.OldValue;

	FHeroValueChangedMessage Message;
	Message.Instigator = GetPawn<APawn>();
	Message.SlotIndex = SlotIndex;
	Message.OldValue = OldValue;
	Message.NewValue = NewValue;
	Message.MaxValue = HealthSet->GetMaxHealth();
	
	const FGameplayTag MessageChannel = ZodiacGameplayTags::GameplayEvent_Damaged_Message;
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(MessageChannel, Message);
}

void UZodiacSkillManagerComponent::HandleUltimateGaugeChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	float NewValue = OnAttributeChangeData.NewValue;
	float OldValue = OnAttributeChangeData.OldValue;

	FHeroValueChangedMessage Message;
	Message.Instigator = GetPawn<APawn>();
	Message.SlotIndex = SlotIndex;
	Message.OldValue = OldValue;
	Message.NewValue = NewValue;
	Message.OptionalValue = RequiredUltimateCostAmount;
	
	const FGameplayTag MessageChannel = UZodiacMessageLibrary::GetUltimateChargeChannel();
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(MessageChannel, Message);
}

void UZodiacSkillManagerComponent::SendResetMessages()
{
	SendSkillSlotChangedMessages();
	
	FOnAttributeChangeData Data;
	Data.NewValue = HealthSet->GetHealth();
	Data.OldValue = HealthSet->GetHealth();
	HandleHealthChanged(Data);
}

void UZodiacSkillManagerComponent::SendHealthBarHeroChangedMessage()
{
	float CurrentHealth = HealthSet->GetHealth();
	float MaxHealth = HealthSet->GetMaxHealth();
	
	//UE_LOG(LogTemp, Warning, TEXT("current health: %.1f, max health: %.1f"), CurrentHealth, MaxHealth);
	FHeroValueChangedMessage Message;
	Message.SlotIndex = SlotIndex;
	Message.Instigator = GetPawn<APawn>();
	Message.NewValue = CurrentHealth;
	Message.MaxValue = MaxHealth;

	const FGameplayTag MessageChannel = ZodiacGameplayTags::Hero_Changed_HealthBar_Message;
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(MessageChannel, Message);
}

void UZodiacSkillManagerComponent::SendSlotStatTagChangedMessage(UZodiacSkillSlot* Slot)
{
	FZodiacSkillCommitMessage_TagStack Message;
	Message.Instigator = GetPawn<APawn>();
	Message.SlotType = Slot->GetSlotType();
	
	// const FZodiacCostEffectData CostData = SkillAbility->GetCostEffectData();
	// if (CostData.AdditionalCosts.Num() > 0)
	// {
	// 	if (UZodiacSkillAbilityCost_TagStack* Cost = Cast<UZodiacSkillAbilityCost_TagStack>(CostData.AdditionalCosts[0]))
	// 	{
	// 		Message.CurrentStack = Slot->GetStatTagStackCount(Cost->TagToSpend());
	// 		Message.ActivationStack = Cost->GetQuantity();
	// 		Message.MaxStack = SlotEntry.SlotDefinition->InitialTagStack.Find(Cost->TagToSpend()) ? SlotEntry.SlotDefinition->InitialTagStack[Cost->TagToSpend()] : 0;
	// 	}
	// }
	//
	// Message.OldValue = OldValue;
	// Message.ConsumeAmount = ChangeAmount;
	
	const FGameplayTag MessageChannel = ZodiacGameplayTags::HUD_Message_SkillCommit;
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(MessageChannel, Message);

	UE_LOG(LogTemp, Warning, TEXT("stat tag message sent"));
}

void UZodiacSkillManagerComponent::SendSkillSlotChangedMessages()
{
	TArray<FGameplayAbilitySpecHandle> Handles;

	for (auto& SlotEntry : SkillSlotList.Entries)
	{
		SlotEntry.Slot->AddStatTagStack(ZodiacGameplayTags::Status_Stun, 1);
		UZodiacSkillSlot* Slot = SlotEntry.Slot;
		const UZodiacSkillSlotFragment_SlotIcon* SlotIcon = Slot->FindFragment<UZodiacSkillSlotFragment_SlotIcon>();

		for (auto& AbilityHandle : SlotEntry.GrantedHandles.AbilitySpecHandles)
		{
			if (FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilityHandle))
			{
				if (UZodiacSkillAbility* SkillAbility = Cast<UZodiacSkillAbility>(AbilitySpec->Ability))
				{
					if (!SkillAbility->GetIsSubordinate())
					{
						FHeroChangedMessage_SkillSlot Message;
						Message.Instigator = GetPawn<APawn>();
						Message.SlotType = Slot->GetSlotType();
						Message.DisplayCostType = SlotEntry.SlotDefinition->DisplayCostTypes;
						Message.Brush = SlotIcon ? SlotIcon->Brush : FSlateBrush();

						const FZodiacCostEffectData CostData = SkillAbility->GetCostEffectData();
						if (CostData.AdditionalCosts.Num() > 0)
						{
							if (UZodiacSkillAbilityCost_TagStack* Cost = Cast<UZodiacSkillAbilityCost_TagStack>(CostData.AdditionalCosts[0]))
							{
								Message.CurrentStack = Slot->GetStatTagStackCount(Cost->TagToSpend());
								Message.ActivationStack = Cost->GetQuantity();
								Message.MaxStack = SlotEntry.SlotDefinition->InitialTagStack.Find(Cost->TagToSpend()) ? SlotEntry.SlotDefinition->InitialTagStack[Cost->TagToSpend()] : 0;
							}
						}
						
						if (SkillAbility->GetCooldownGameplayEffect())
						{
							GetCooldown2(Message, Slot->GetSlotType());
						}

						const FGameplayTag MessageChannel = ZodiacGameplayTags::HUD_Message_HeroChanged_SkillSlot;
						UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
						MessageSubsystem.BroadcastMessage(MessageChannel, Message);
					}
				}
			}
		}
	}
	
	// AbilitySystemComponent->GetAllAbilities(Handles);
	// for (auto& Handle : Handles)
	// {
	// 	if (FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle))
	// 	{
	// 		if (UZodiacSkillSlot* SkillSlot = Cast<UZodiacSkillSlot>(AbilitySpec->SourceObject))
	// 		{
	// 			const UZodiacSkillSlotFragment_SlotIcon* SlotIcon = SkillSlot->FindFragment<UZodiacSkillSlotFragment_SlotIcon>();
	// 			FGameplayTag SlotType = SkillSlot->GetSlotType();
	// 			if (SlotType.IsValid())
	// 			{
	// 				FHeroChangedMessage_SkillSlot Message;
	// 				Message.Instigator = GetPawn<APawn>();
	// 				Message.SlotType = SlotType;
	// 				Message.Brush = SlotIcon ? SlotIcon->Brush : FSlateBrush();
	// 				
	// 				GetCooldown(Message, SlotType);
	// 				
	// 				const FGameplayTag MessageChannel = ZodiacGameplayTags::HUD_Message_HeroChanged_SkillSlot;
	// 				UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	// 				MessageSubsystem.BroadcastMessage(MessageChannel, Message);
	// 			}
	// 		}
	// 	}
	// }

	if (!HasAuthority())
	{
		//UE_LOG(LogTemp, Warning, TEXT("on client"));
		for (auto& SlotEntry : SkillSlotList.Entries)
		{
			if (SlotEntry.Slot)
			{
				//UE_LOG(LogTemp, Warning, TEXT("slot: %s"), *SlotEntry.Slot.GetName());
			}
		}
	}

	AbilitySystemComponent->GetAllAbilities(Handles);
	for (auto& Handle : Handles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle))
		{
			if (UZodiacSkillSlot* SkillSlot = Cast<UZodiacSkillSlot>(AbilitySpec->SourceObject))
			{
				//UE_LOG(LogTemp, Warning, TEXT("slot from sourceobject: %s"), *SkillSlot->GetName());
			}
		}
	}
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
		OutMessage.CurrentValue = DurationAndTimeRemaining[BestIdx].Key;
		OutMessage.MaxValue = DurationAndTimeRemaining[BestIdx].Value;
		OutMessage.bIsReady = false;
		UE_LOG(LogTemp, Warning, TEXT("cool down: %.1f, remaining: %.1f"), OutMessage.MaxValue, OutMessage.CurrentValue );
	}
	else
	{
		OutMessage.CurrentValue = 0;
		OutMessage.MaxValue = BIG_NUMBER;
		OutMessage.bIsReady = true;
	}
}

void UZodiacSkillManagerComponent::GetCooldown2(FHeroChangedMessage_SkillSlot& OutMessage, FGameplayTag SlotType)
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
		UE_LOG(LogTemp, Warning, TEXT("cool down: %.1f, remaining: %.1f"), OutMessage.MaxValue, OutMessage.CurrentValue );
	}
	else
	{
		OutMessage.Cooldown_Remaining = 0;
		OutMessage.Cooldown_Duration = BIG_NUMBER;
		OutMessage.bIsReady = true;
	}
}

void UZodiacSkillManagerComponent::OnRep_SkillSlotList(const FZodiacSkillSlotList& OldList)
{
	UE_LOG(LogTemp, Warning, TEXT("onrepskillslotlist"));
}

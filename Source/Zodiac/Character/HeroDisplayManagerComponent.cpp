// the.quiet.string@gmail.com


#include "HeroDisplayManagerComponent.h"

#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"
#include "AbilitySystem/Attributes/ZodiacUltimateSet.h"
#include "AbilitySystem/Skills/ZodiacSkillDefinition.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/ZodiacMessageLibrary.h"
#include "Messages/ZodiacMessageTypes.h"


UHeroDisplayManagerComponent::UHeroDisplayManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	AbilitySystemComponent = nullptr;
	HealthSet = nullptr;
	UltimateSet = nullptr;
}

void UHeroDisplayManagerComponent::InitializeHeroData(const int32 InSlotIndex, UZodiacAbilitySystemComponent* InZodiacASC, const TArray<UZodiacSkillDefinition*>
                                                      & InSkillDefinitions, FSimpleMulticastDelegate& OnHeroChanged)
{
	check(InZodiacASC);

	SlotIndex = InSlotIndex;
	AbilitySystemComponent = InZodiacASC;
	SkillDefinitions = InSkillDefinitions;
	OnHeroChanged.AddUObject(this, &ThisClass::OnHeroChanged);
	
	for (auto& Skill : SkillDefinitions)
	{
		float CostAmount = AbilitySystemComponent->GetRequiredSkillCostAmount(Skill->SkillID);
		RequiredCostAmounts.Add(Skill->SkillID, CostAmount);

		FGameplayTag CostType;
		if (AbilitySystemComponent->FindSkillCostType(Skill->SkillID, OUT CostType))
		{
			SkillCostTypeMap.Add(Skill->SkillID, OUT CostType);

			if (CostType == ZodiacGameplayTags::Ability_Type_Skill_Cost_Ultimate)
			{
				RequiredUltimateCostAmount = RequiredCostAmounts[Skill->SkillID];
			}
		}
	}
	
	HealthSet = CastChecked<UZodiacHealthSet>(AbilitySystemComponent->GetAttributeSet(UZodiacHealthSet::StaticClass()));
	//CombatSet = CastChecked<UZodiacCombatSet>(InASC->GetAttributeSet(UZodiacCombatSet::StaticClass()));
	UltimateSet = CastChecked<UZodiacUltimateSet>(AbilitySystemComponent->GetAttributeSet(UZodiacUltimateSet::StaticClass()));
	
	InZodiacASC->GetGameplayAttributeValueChangeDelegate(UZodiacHealthSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	InZodiacASC->GetGameplayAttributeValueChangeDelegate(UZodiacUltimateSet::GetUltimateGaugeAttribute()).AddUObject(this, &ThisClass::HandleUltimateGaugeChanged);

	if (InSlotIndex == 0)
	{
		SendResetMessages();	
	}
}

void UHeroDisplayManagerComponent::OnHeroChanged()
{
	SendSkillChangedMessages();
	SendHealthBarHeroChangedMessage();
}

void UHeroDisplayManagerComponent::HandleHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
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

void UHeroDisplayManagerComponent::HandleUltimateGaugeChanged(const FOnAttributeChangeData& OnAttributeChangeData)
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

void UHeroDisplayManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHeroDisplayManagerComponent::SendResetMessages()
{
	SendSkillChangedMessages();
	
	FOnAttributeChangeData Data;
	Data.NewValue = HealthSet->GetHealth();
	Data.OldValue = HealthSet->GetHealth();
	HandleHealthChanged(Data);
}

void UHeroDisplayManagerComponent::SendHealthBarHeroChangedMessage()
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

void UHeroDisplayManagerComponent::SendSkillChangedMessages()
{
	for (auto& Skill : SkillDefinitions)
	{
		//UE_LOG(LogTemp, Warning, TEXT("skill match found: %s, %s"), *Skill->SkillID.ToString(), HasAuthority() ? TEXT("server") : TEXT("client"));
		FGameplayTag SlotType;
		if (AbilitySystemComponent->SkillHandles.FindSlotType(Skill->SkillID, OUT SlotType))
		{
			FHeroChangedMessage_SkillSlot Message;
			Message.Instigator = GetPawn<APawn>();
			Message.Brush = Skill->Brush;

			if (SlotType == ZodiacGameplayTags::Ability_Type_Skill_Slot_Primary)
			{
				
			}
			else if (SlotType == ZodiacGameplayTags::Ability_Type_Skill_Slot_Secondary)
			{
				GetCooldown(Message, Skill->SkillID);
			}
			else if (SlotType == ZodiacGameplayTags::Ability_Type_Skill_Slot_Ultimate)
			{
				GetUltimateGauge(Message);
			}
			
			const FGameplayTag MessageChannel = UZodiacMessageLibrary::GetSkillChangeChannelByTag(SlotType);
			UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSubsystem.BroadcastMessage(MessageChannel, Message);	
		}
	}
}

void UHeroDisplayManagerComponent::GetUltimateGauge(FHeroChangedMessage_SkillSlot& OutMessage)
{
	OutMessage.CurrentValue = UltimateSet->GetUltimateGauge();
	OutMessage.MaxValue = UltimateSet->GetMaxUltimateGauge();
	OutMessage.OptionalValue = RequiredUltimateCostAmount;
}

void UHeroDisplayManagerComponent::GetCooldown(FHeroChangedMessage_SkillSlot& OutMessage, FGameplayTag SkillID)
{
	FGameplayTag SlotType;
	if (!AbilitySystemComponent->FindSkillSlotType(SkillID, SlotType))
	{
		return;
	}
	
	FGameplayTagContainer QueryContainer;
	QueryContainer.AddTag(ZodiacGameplayTags::GetCooldownExtendedTag(SlotType));
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
		//UE_LOG(LogTemp, Warning, TEXT("cool down: %.1f, remaining: %.1f"), OutMessage.MaxValue, OutMessage.CurrentValue );
	}
	else
	{
		OutMessage.CurrentValue = 0;
		OutMessage.MaxValue = BIG_NUMBER;
		OutMessage.bIsReady = true;
	}
}

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
                                                      & InSkillDefinitions)
{
	check(InZodiacASC);

	SlotIndex = InSlotIndex;
	AbilitySystemComponent = InZodiacASC;
	SkillDefinitions = InSkillDefinitions;
	
	HealthSet = CastChecked<UZodiacHealthSet>(InZodiacASC->GetAttributeSet(UZodiacHealthSet::StaticClass()));
	//CombatSet = CastChecked<UZodiacCombatSet>(InASC->GetAttributeSet(UZodiacCombatSet::StaticClass()));
	UltimateSet = CastChecked<UZodiacUltimateSet>(InZodiacASC->GetAttributeSet(UZodiacUltimateSet::StaticClass()));
	
	InZodiacASC->GetGameplayAttributeValueChangeDelegate(HealthSet->GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	InZodiacASC->GetGameplayAttributeValueChangeDelegate(UltimateSet->GetUltimateGaugeAttribute()).AddUObject(this, &ThisClass::HandleUltimateGaugeChanged);
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
	Message.Instigator = Cast<APawn>(GetOwner());
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
	
	FUltimateChargeMessage Message;
	Message.Instigator = GetOwner();
	Message.ChargeAmount = NewValue;
	
	const FGameplayTag MessageChannel = UZodiacMessageLibrary::GetUltimateChargeChannel();
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(MessageChannel, Message);
}

void UHeroDisplayManagerComponent::SendHealthBarHeroChangedMessage()
{
	const UZodiacHealthSet* HealthSet_Temp = Cast<UZodiacHealthSet>(AbilitySystemComponent->GetAttributeSet(UZodiacHealthSet::StaticClass()));
	float CurrentHealth = HealthSet_Temp->GetHealth();
	float MaxHealth = HealthSet_Temp->GetMaxHealth();
	
	UE_LOG(LogTemp, Warning, TEXT("current health: %.1f, max health: %.1f"), CurrentHealth, MaxHealth);
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
	// currently not working only for code reference.
	// for (auto& [Handle, Skill] : SkillMap)
	// {
	// 	FHeroChangedMessage_SkillSlot Message;
	// 	Message.Instigator = GetPawn<APawn>();
	// 	Message.Brush = Skill.GetFragmentByClass<USkillFragment_Display>()->Brush;
	//
	// 	if (Skill.SlotType == ZodiacGameplayTags::Ability_Type_Skill_Slot_Primary)
	// 	{
	// 		
	// 	}
	// 	else if (Skill.SlotType == ZodiacGameplayTags::Ability_Type_Skill_Slot_Secondary)
	// 	{
	// 		GetCooldown(Message, &Skill);
	// 	}
	// 	else if (Skill.SlotType == ZodiacGameplayTags::Ability_Type_Skill_Slot_Ultimate)
	// 	{
	// 		GetUltimateGauge(Message, &Skill);
	// 	}
	// 		
	// 	const FGameplayTag MessageChannel = UZodiacMessageLibrary::GetSkillChangeChannelByTag(Skill.SlotType);
	// 	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	// 	MessageSubsystem.BroadcastMessage(MessageChannel, Message);
	// }
	
	for (auto& Skill : SkillDefinitions)
	{
		UE_LOG(LogTemp, Warning, TEXT("skill match found: %s, %s"), *Skill->SkillID.ToString(), HasAuthority() ? TEXT("server") : TEXT("client"));
		FGameplayTag SlotType;
		if (AbilitySystemComponent->SkillHandles.GetSlotType(Skill->SkillID, OUT SlotType))
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
	UE_LOG(LogTemp, Warning, TEXT("ult gauge current: %.1f: , max: %.1f"), UltimateSet->GetUltimateGauge(), UltimateSet->GetMaxUltimateGauge());
	OutMessage.CurrentValue = UltimateSet->GetUltimateGauge();
	OutMessage.MaxValue = UltimateSet->GetMaxUltimateGauge();
}

void UHeroDisplayManagerComponent::GetCooldown(FHeroChangedMessage_SkillSlot& OutMessage, FGameplayTag SkillID)
{
	FGameplayTagContainer QueryContainer;

	FGameplayTag SlotType;
	if (!AbilitySystemComponent->SkillHandles.GetSlotType(SkillID, SlotType))
	{
		return;
	}
	
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

		UE_LOG(LogTemp, Warning, TEXT("cool down: %.1f, remaining: %.1f"), OutMessage.MaxValue, OutMessage.CurrentValue );
	}
	else
	{
		OutMessage.CurrentValue = 0;
		OutMessage.MaxValue = BIG_NUMBER;
		OutMessage.bIsReady = true;
	}
}

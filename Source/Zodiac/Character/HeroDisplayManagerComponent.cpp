// the.quiet.string@gmail.com


#include "HeroDisplayManagerComponent.h"

#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"
#include "AbilitySystem/Attributes/ZodiacUltimateSet.h"
#include "GameFramework/Character.h"
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

void UHeroDisplayManagerComponent::InitializeHeroData(const int32 InSlotIndex, UAbilitySystemComponent* InASC)
{
	check(InASC);

	SlotIndex = InSlotIndex;
	AbilitySystemComponent = InASC;
	
	HealthSet = CastChecked<UZodiacHealthSet>(InASC->GetAttributeSet(UZodiacHealthSet::StaticClass()));
	//CombatSet = CastChecked<UZodiacCombatSet>(InASC->GetAttributeSet(UZodiacCombatSet::StaticClass()));
	UltimateSet = CastChecked<UZodiacUltimateSet>(InASC->GetAttributeSet(UZodiacUltimateSet::StaticClass()));
	
	InASC->GetGameplayAttributeValueChangeDelegate(HealthSet->GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	InASC->GetGameplayAttributeValueChangeDelegate(UltimateSet->GetUltimateGaugeAttribute()).AddUObject(this, &ThisClass::HandleUltimateGaugeChanged);
}

void UHeroDisplayManagerComponent::RegisterSkillDisplayData(const FZodiacSkillSetWithHandle& SkillData)
{
	for (const auto& Elem : SkillData.Map)
	{
		FGameplayAbilitySpecHandle SpecHandle = Elem.Key;
		const FZodiacSkillSet* SkillSet = Elem.Value;
		SkillMap.Add(SpecHandle, *SkillSet);
	}
}

void UHeroDisplayManagerComponent::OnHeroChanged()
{
	SendSkillChangedMessages();
	SendHealthBarHeroChangedMessage();
}

void UHeroDisplayManagerComponent::OnSkillChanged(UAbilitySystemComponent* InASC,
                                                  const TArray<FGameplayAbilitySpecHandle>& Handles)
{
	SendSkillChangedMessages();
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
	if (!HasAuthority() && GetPawn<ACharacter>()->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("skill changed message"));	
	}
	
	for (auto& [Handle, Skill] : SkillMap)
	{
		FHeroChangedMessage_SkillSlot Message;
		Message.Instigator = GetPawn<APawn>();
		Message.Brush = Skill.GetFragmentByClass<USkillFragment_Display>()->Brush;
		
		if (!HasAuthority() && GetPawn<ACharacter>()->IsLocallyControlled())
		{
			UE_LOG(LogTemp, Warning, TEXT("skill type: %s"), *Skill.SlotType.GetTagName().ToString());	
		}
		
		if (Skill.SlotType == ZodiacGameplayTags::Ability_Type_Skill_Slot_Primary)
		{
			
		}
		else if (Skill.SlotType == ZodiacGameplayTags::Ability_Type_Skill_Slot_Secondary)
		{
			GetCooldown(Message, &Skill);
		}
		else if (Skill.SlotType == ZodiacGameplayTags::Ability_Type_Skill_Slot_Ultimate)
		{
			GetUltimateGauge(Message, &Skill);
		}
			
		const FGameplayTag MessageChannel = UZodiacMessageLibrary::GetSkillChangeChannelByTag(Skill.SlotType);
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSubsystem.BroadcastMessage(MessageChannel, Message);
	}
}

void UHeroDisplayManagerComponent::GetUltimateGauge(FHeroChangedMessage_SkillSlot& OutMessage, FZodiacSkillSet* Skill)
{
	UE_LOG(LogTemp, Warning, TEXT("ult gauge current: %.1f: , max: %.1f"), UltimateSet->GetUltimateGauge(), UltimateSet->GetMaxUltimateGauge());
	OutMessage.CurrentValue = UltimateSet->GetUltimateGauge();
	OutMessage.MaxValue = UltimateSet->GetMaxUltimateGauge();
}

void UHeroDisplayManagerComponent::GetCooldown(FHeroChangedMessage_SkillSlot& OutMessage, FZodiacSkillSet* Skill)
{
	FGameplayTagContainer QueryContainer;
	QueryContainer.AddTag(ZodiacGameplayTags::GetCooldownExtendedTag(Skill->SlotType));
	
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

// the.quiet.string@gmail.com


#include "ZodiacSkillManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/Attributes/ZodiacUltimateSet.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/ZodiacMessageLibrary.h"

void UZodiacSkillManagerComponent::RegisterSkillDisplayData(const FZodiacSkillSetWithHandle& SkillData)
{
	for (const auto& Elem : SkillData.Map)
	{
		FGameplayAbilitySpecHandle SpecHandle = Elem.Key;
		const FZodiacSkillSet* SkillSet = Elem.Value;
		SkillMap.Add(SpecHandle, *SkillSet);
	}
}

void UZodiacSkillManagerComponent::HandleSkillChanged(UAbilitySystemComponent* InASC,
                                                      const TArray<FGameplayAbilitySpecHandle>& Handles)
{
	for (auto& Handle : Handles)
	{
		if (FZodiacSkillSet* Skill = SkillMap.Find(Handle))
		{
			FHeroChangedMessage_SkillSlot Message;
			Message.Instigator = GetPawn<APawn>();
			Message.Brush = Skill->GetFragmentByClass<USkillFragment_Display>()->Brush;
			
			if (Skill->SlotType == ZodiacGameplayTags::Ability_Type_Skill_Slot_Primary)
			{
				
			}
			else if (Skill->SlotType == ZodiacGameplayTags::Ability_Type_Skill_Slot_Secondary)
			{
				GetCooldown(Message, InASC, Skill);
			}
			else if (Skill->SlotType == ZodiacGameplayTags::Ability_Type_Skill_Slot_Ultimate)
			{
				GetUltimateGauge(Message, InASC, Skill);
			}
			
			const FGameplayTag MessageChannel = UZodiacMessageLibrary::GetSkillChangeChannelByTag(Skill->SlotType);
			UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSubsystem.BroadcastMessage(MessageChannel, Message);
		}
	}
}

void UZodiacSkillManagerComponent::GetUltimateGauge(FHeroChangedMessage_SkillSlot& OutMessage,
                                                  UAbilitySystemComponent* InASC, FZodiacSkillSet* Skill)
{
	const UZodiacUltimateSet* UltimateSet = CastChecked<UZodiacUltimateSet>(InASC->GetAttributeSet(UZodiacUltimateSet::StaticClass()));
	UE_LOG(LogTemp, Warning, TEXT("ult gauge current: %.1f: , max: %.1f"), UltimateSet->GetUltimateGauge(), UltimateSet->GetMaxUltimateGauge());
	OutMessage.CurrentValue = UltimateSet->GetUltimateGauge();
	OutMessage.MaxValue = UltimateSet->GetMaxUltimateGauge();
}

void UZodiacSkillManagerComponent::GetCooldown(FHeroChangedMessage_SkillSlot& OutMessage,
                                               UAbilitySystemComponent* InASC,
                                               FZodiacSkillSet* Skill)
{
	check(InASC);

	FGameplayTagContainer QueryContainer;
	QueryContainer.AddTag(GetCooldownExtendedTag(Skill->SlotType));
	
	FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(QueryContainer);
	TArray< TPair<float,float> > DurationAndTimeRemaining = InASC->GetActiveEffectsTimeRemainingAndDuration(Query);
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

FGameplayTag UZodiacSkillManagerComponent::GetCooldownExtendedTag(const FGameplayTag& SkillTag)
{
	FString CooldownTagString = SkillTag.ToString() + TEXT(".") + TEXT("Cooldown");
	return FGameplayTag::RequestGameplayTag(*CooldownTagString);
}

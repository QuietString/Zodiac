// the.quiet.string@gmail.com


#include "ZodiacSkillManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "GameFramework/GameplayMessageSubsystem.h"


void UZodiacSkillManagerComponent::InitializeWithHeroComponent(UZodiacHeroComponent* HeroComponent)
{
	
}

void UZodiacSkillManagerComponent::RegisterSkillDisplayData(const FZodiacSkillSetWithHandle& SkillData)
{
	for (const auto& Elem : SkillData.Map)
	{
		FGameplayAbilitySpecHandle SpecHandle = Elem.Key;
		const FZodiacSkillSet* SkillSet = Elem.Value;
		USkillFragment_Display* Fragment_Display = SkillSet->GetFragmentByClass<USkillFragment_Display>();

		FSkillDisplayData SkillDisplayData;
		SkillDisplayData.Handle = SpecHandle;
		SkillDisplayData.Brush = Fragment_Display->Brush;
		SkillDisplayDataList.Add(SkillDisplayData);
	}
}

void UZodiacSkillManagerComponent::HandleSkillChanged(UAbilitySystemComponent* InASC,
                                                     const TArray<FGameplayAbilitySpecHandle>& Handles)
{
	for (auto& Handle : Handles)
	{
		OnSkillChanged(InASC, Handle);
	}
}

void UZodiacSkillManagerComponent::OnSkillChanged(UAbilitySystemComponent* InASC, const FGameplayAbilitySpecHandle& SpecHandle)
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());

	float OutCooldownRemaining;
	float OutCooldownDuration;
	bool bCooldownFound = GetCooldown(InASC, SpecHandle, OutCooldownRemaining, OutCooldownDuration);
	
	FHeroChangedMessage_SkillSlot Message_SkillSlot;
	Message_SkillSlot.PlayerPawn = GetPawn<APawn>();
	Message_SkillSlot.Brush = SkillDisplayDataList.FindByPredicate([SpecHandle](const FSkillDisplayData& DisplayData)
	{
		return DisplayData==SpecHandle;
	})->Brush;
	Message_SkillSlot.Cooldown_Duration = OutCooldownDuration;
	Message_SkillSlot.Cooldown_Remaining = OutCooldownRemaining;

	UE_LOG(LogTemp, Warning, TEXT("cool down: %.1f, remaining: %.1f"), OutCooldownDuration, OutCooldownRemaining);

	MessageSubsystem.BroadcastMessage(ZodiacGameplayTags::HUD_Slot_HeroChanged_Message, Message_SkillSlot);
}

bool UZodiacSkillManagerComponent::GetCooldown(UAbilitySystemComponent* InASC, const FGameplayAbilitySpecHandle Handle, float& CooldownRemaining,
                                               float& CooldownDuration)
{
	FGameplayAbilitySpec* AbilitySpec = InASC->FindAbilitySpecFromHandle(Handle);
	if (UZodiacGameplayAbility* ZodiacAbility = Cast<UZodiacGameplayAbility>(AbilitySpec->Ability))
	{
		FGameplayTagContainer QueryContainer;
		QueryContainer.AppendTags(*ZodiacAbility->GetCooldownTags());
		//QueryContainer.AddTag(*ZodiacAbility->GetSkillTag());
		
		FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAllOwningTags(QueryContainer);
		TArray< TPair<float, float> > DurationAndTimeRemaining = InASC->GetActiveEffectsTimeRemainingAndDuration(Query);
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
			CooldownRemaining = DurationAndTimeRemaining[BestIdx].Key;
			CooldownDuration = DurationAndTimeRemaining[BestIdx].Value;
			
			return true;
		}
	}
	
	return false;
}

// the.quiet.string@gmail.com


#include "ZodiacSkillManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/ZodiacMessageLibrary.h"

void UZodiacSkillManagerComponent::RegisterSkillDisplayData(const FZodiacSkillSetWithHandle& SkillData)
{
	for (const auto& Elem : SkillData.Map)
	{
		FGameplayAbilitySpecHandle SpecHandle = Elem.Key;
		const FZodiacSkillSet* SkillSet = Elem.Value;
		USkillFragment_Display* Fragment_Display = SkillSet->GetFragmentByClass<USkillFragment_Display>();

		FSkillDataForDisplay Data;
		Data.SkillName = Fragment_Display->DisplayName;
		Data.Brush = Fragment_Display->Brush;
		Data.SkillTag = SkillSet->SkillType;
		Data.CooldownTag = GetCooldownExtendedTag(SkillSet->SkillType);
		DisplayDataMap.Add(SpecHandle, Data);
	}
}

void UZodiacSkillManagerComponent::HandleSkillChanged(UAbilitySystemComponent* InASC,
                                                     const TArray<FGameplayAbilitySpecHandle>& Handles)
{
	for (auto& Handle : Handles)
	{
		if (FSkillDataForDisplay* Data = DisplayDataMap.Find(Handle))
		{
			OnSkillChanged(InASC, Handle, Data->SkillTag);
		}
	}
}

void UZodiacSkillManagerComponent::OnSkillChanged(UAbilitySystemComponent* InASC, const FGameplayAbilitySpecHandle& SpecHandle, const FGameplayTag& SkillType)
{	
	const FGameplayTag MessageChannel = UZodiacMessageLibrary::GetSkillChangeChannelByTag(SkillType);

	float OutCooldownRemaining;
	float OutCooldownDuration;
	bool bCooldownFound = GetCooldown(InASC, SpecHandle, OutCooldownRemaining, OutCooldownDuration);
	
	FHeroChangedMessage_SkillSlot Message_SkillSlot;
	Message_SkillSlot.Instigator = GetPawn<APawn>();
	Message_SkillSlot.HeroName = DisplayDataMap.Find(SpecHandle)->SkillName;
	Message_SkillSlot.Brush = DisplayDataMap.Find(SpecHandle)->Brush;
	Message_SkillSlot.HaveCooldown = bCooldownFound;
	Message_SkillSlot.Cooldown_Duration = OutCooldownDuration;
	Message_SkillSlot.Cooldown_Remaining =  OutCooldownRemaining ;
	
	UE_LOG(LogTemp, Warning, TEXT("cool down: %.1f, remaining: %.1f"), OutCooldownDuration, OutCooldownRemaining);

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(MessageChannel, Message_SkillSlot);
}

bool UZodiacSkillManagerComponent::GetCooldown(UAbilitySystemComponent* InASC, const FGameplayAbilitySpecHandle Handle, float& CooldownRemaining,
                                               float& CooldownDuration)
{
	check(InASC);
	
	CooldownRemaining = 0.0f;
	CooldownDuration = BIG_NUMBER;
	
	FGameplayTagContainer QueryContainer;
	QueryContainer.AddTag(DisplayDataMap.Find(Handle)->CooldownTag);
	
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
		CooldownRemaining = DurationAndTimeRemaining[BestIdx].Key;
		CooldownDuration = DurationAndTimeRemaining[BestIdx].Value;

		return true;
	}
	
	return false;
}

FGameplayTag UZodiacSkillManagerComponent::GetCooldownExtendedTag(const FGameplayTag& SkillTag)
{
	FString CooldownTagString = SkillTag.ToString() + TEXT(".") + TEXT("Cooldown");
	return FGameplayTag::RequestGameplayTag(*CooldownTagString);
}

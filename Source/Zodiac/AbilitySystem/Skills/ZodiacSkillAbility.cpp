// the.quiet.string@gmail.com


#include "AbilitySystem/Skills/ZodiacSkillAbility.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacSkillSlot.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/ZodiacMessageTypes.h"


UZodiacSkillSlot* UZodiacSkillAbility::GetSkillSlot() const
{
	if (FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
	{
		return Cast<UZodiacSkillSlot>(Spec->SourceObject.Get());
	}

	return nullptr;
}

const FGameplayTagContainer* UZodiacSkillAbility::GetCooldownTags() const
{
	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&TempCooldownTags);
	MutableTags->Reset();
	
	if (UZodiacSkillSlot* SkillSlot = GetSkillSlot())
	{
		MutableTags->AddTag(SkillSlot->GetSlotType());
	}
	
	return MutableTags;
}

void UZodiacSkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bIsFirstActivation = true;
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UZodiacSkillAbility::CommitExecute(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::CommitExecute(Handle, ActorInfo, ActivationInfo);

	if (!CheckCooldown(Handle, CurrentActorInfo))
	{
		SendCooldownMessage();	
	}
	
	bIsFirstActivation = false;
}

void UZodiacSkillAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (UGameplayEffect* CooldownGE = GetCooldownGameplayEffect())
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		if (FGameplayEffectSpec* EffectSpec = SpecHandle.Data.Get())
		{
			if (UZodiacSkillSlot* SkillSlot = Cast<UZodiacSkillSlot>(GetSourceObject(Handle, CurrentActorInfo)))
			{
				EffectSpec->DynamicGrantedTags.AddTag(SkillSlot->GetSlotType());
			}
		
			EffectSpec->SetSetByCallerMagnitude(ZodiacGameplayTags::SetByCaller_Cooldown, CooldownDuration.GetValueAtLevel(GetAbilityLevel()));
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}
	
	Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
}

float UZodiacSkillAbility::GetCurrentCostAmount() const
{
	if (CostData.bUseSeparateMidActivationCost && !bIsFirstActivation)
	{
		return CostData.MidActivationCostAmount.GetValue();
	}
	
	return CostData.ActivationCostAmount.GetValue();
}

float UZodiacSkillAbility::GetInitialActivationCost() const
{
	return CostData.ActivationCostAmount.GetValue();
}

void UZodiacSkillAbility::SendCooldownMessage()
{
	FSkillDurationMessage Message;
	Message.Instigator = CurrentActorInfo->OwnerActor.Get();
	Message.Cooldown_Duration = GetCooldownDuration();
	if (UZodiacSkillSlot* SkillSlot = GetSkillSlot())
	{
		Message.SlotType = SkillSlot->GetSlotType();	
	}
	
	const FGameplayTag MessageChannel = ZodiacGameplayTags::HUD_Message_SkillDuration;
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(MessageChannel, Message);
}

void UZodiacSkillAbility::ChargeUltimate()
{
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(ChargeUltimateEffect, GetAbilityLevel());
	EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(ZodiacGameplayTags::SetByCaller_UltimateGauge, UltimateChargeAmount.GetValueAtLevel(GetAbilityLevel()));

	ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle);
}
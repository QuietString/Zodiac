// the.quiet.string@gmail.com


#include "AbilitySystem/Skills/ZodiacSkillAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacSkillSlot.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/ZodiacSkillAbilityCost.h"
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

	if (CooldownGameplayEffectClass)
	{
		SendCooldownMessage();	
	}
	
	bIsFirstActivation = false;
}

bool UZodiacSkillAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
	{
		return false;
	}

	// Verify we can afford any additional costs
	for (TObjectPtr<UZodiacSkillAbilityCost> AdditionalCost : CostData.GetCurrentAdditionalCostData(bIsFirstActivation))
	{
		if (AdditionalCost != nullptr)
		{
			if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, OUT OptionalRelevantTags))
			{
				return false;
			}
		}
	}

	return true;
}

void UZodiacSkillAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
	
	// Used to determine if the ability actually hit a target (as some costs are only spent on successful attempts)
	auto DetermineIfAbilityHitTarget = [&]()
	{
		if (ActorInfo->IsNetAuthority())
		{
			if (UZodiacAbilitySystemComponent* ASC = Cast<UZodiacAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
			{
				FGameplayAbilityTargetDataHandle TargetData;
				ASC->GetAbilityTargetData(Handle, ActivationInfo, TargetData);
				for (int32 TargetDataIdx = 0; TargetDataIdx < TargetData.Data.Num(); ++TargetDataIdx)
				{
					if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, TargetDataIdx))
					{
						return true;
					}
				}
			}
		}

		return false;
	};
	
	// Pay any additional costs
	bool bAbilityHitTarget = false;
	bool bHasDeterminedIfAbilityHitTarget = false;
	for (TObjectPtr<UZodiacSkillAbilityCost> AdditionalCost : CostData.GetCurrentAdditionalCostData(bIsFirstActivation))
	{
		if (AdditionalCost != nullptr)
		{
			if (AdditionalCost->ShouldOnlyApplyCostOnHit())
			{
				if (!bHasDeterminedIfAbilityHitTarget)
				{
					bAbilityHitTarget = DetermineIfAbilityHitTarget();
					bHasDeterminedIfAbilityHitTarget = true;
				}

				if (!bAbilityHitTarget)
				{
					continue;
				}
			}

			AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
		}
	}
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

float UZodiacSkillAbility::GetCurrentGECostAmount() const
{
	return CostData.GetCurrentCostGEAmount(bIsFirstActivation).GetValue();
}

TArray<TObjectPtr<UZodiacSkillAbilityCost>> UZodiacSkillAbility::GetCurrentAdditionalCosts()
{
	return CostData.GetCurrentAdditionalCostData(bIsFirstActivation);
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
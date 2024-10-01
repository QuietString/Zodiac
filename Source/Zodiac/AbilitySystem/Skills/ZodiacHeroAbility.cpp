// the.quiet.string@gmail.com

#include "AbilitySystem/Skills/ZodiacHeroAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacSkillSlot.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/ZodiacSkillAbilityCost.h"
#include "Character/ZodiacHeroActor.h"
#include "Character/ZodiacHostCharacter.h"
#include "Player/ZodiacPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAbility)

UAbilitySystemComponent* UZodiacHeroAbility::GetHostAbilitySystemComponent() const
{
	if (CurrentActorInfo)
	{
		if (AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(CurrentActorInfo->OwnerActor.Get()))
		{
			return HostCharacter->GetHostAbilitySystemComponent();
		}
	}

	return nullptr;
}

AZodiacPlayerController* UZodiacHeroAbility::GetHostPlayerControllerFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AZodiacPlayerController>(GetZodiacHostCharacterFromActorInfo()->GetController()) : nullptr);
}

AZodiacHeroActor* UZodiacHeroAbility::GetHeroActorFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AZodiacHeroActor>(GetAvatarActorFromActorInfo()) : nullptr);
}

UZodiacSkillSlot* UZodiacHeroAbility::GetSkillSlot() const
{
	if (FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
	{
		return Cast<UZodiacSkillSlot>(Spec->SourceObject.Get());
	}

	return nullptr;
}

const FGameplayTagContainer* UZodiacHeroAbility::GetCooldownTags() const
{
	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&TempCooldownTags);
	MutableTags->Reset();
	
	if (UZodiacSkillSlot* SkillSlot = GetSkillSlot())
	{
		MutableTags->AddTag(SkillSlot->GetSlotType());
	}
	
	return MutableTags;
}

bool UZodiacHeroAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (UAbilitySystemComponent* HostASC = GetHostAbilitySystemComponent())
	{
		return DoesAbilitySatisfyTagRequirements(*HostASC, SourceTags, TargetTags, OptionalRelevantTags);	
	}
	
	return true;
}

void UZodiacHeroAbility::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
	const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);

	if (UAbilitySystemComponent* HostASC = GetHostAbilitySystemComponent())
	{
		HostASC->AddLooseGameplayTags(ActivationOwnedTags);
	}
}

void UZodiacHeroAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	bIsFirstActivation = true;

	if (AimingEffect && bAimWhenActivated)
	{
		ApplyAimingEffect();
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UZodiacHeroAbility::CommitExecute(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::CommitExecute(Handle, ActorInfo, ActivationInfo);

	bIsFirstActivation = false;
}

bool UZodiacHeroAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
	{
		return false;
	}
	//
	// // Verify we can afford any additional costs
	// for (TObjectPtr<UZodiacSkillAbilityCost> AdditionalCost : CostData.GetCurrentAdditionalCostData(bIsFirstActivation))
	// {
	// 	if (AdditionalCost != nullptr)
	// 	{
	// 		if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, OUT OptionalRelevantTags))
	// 		{
	// 			return false;
	// 		}
	// 	}
	// }

	return true;
}

void UZodiacHeroAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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
	// bool bAbilityHitTarget = false;
	// bool bHasDeterminedIfAbilityHitTarget = false;
	// for (TObjectPtr<UZodiacSkillAbilityCost> AdditionalCost : CostData.GetCurrentAdditionalCostData(bIsFirstActivation))
	// {
	// 	if (AdditionalCost != nullptr)
	// 	{
	// 		if (AdditionalCost->ShouldOnlyApplyCostOnHit())
	// 		{
	// 			if (!bHasDeterminedIfAbilityHitTarget)
	// 			{
	// 				bAbilityHitTarget = DetermineIfAbilityHitTarget();
	// 				bHasDeterminedIfAbilityHitTarget = true;
	// 			}
	//
	// 			if (!bAbilityHitTarget)
	// 			{
	// 				continue;
	// 			}
	// 		}
	//
	// 		AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
	// 	}
	// }
}

void UZodiacHeroAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
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

void UZodiacHeroAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (UAbilitySystemComponent* HostASC = GetHostAbilitySystemComponent())
	{
		HostASC->RemoveLooseGameplayTags(ActivationOwnedTags);
	}
}

FVector UZodiacHeroAbility::GetWeaponLocation() const
{
	if (USkeletalMeshComponent* MeshComponent =  GetOwningComponentFromActorInfo())
	{
		if (ComboSockets.IsValidIndex(ComboIndex))
		{
			return MeshComponent->GetSocketLocation(ComboSockets[ComboIndex]);	
		}

		return MeshComponent->GetSocketLocation(FName());
	}

	return  FVector();
}

void UZodiacHeroAbility::ApplyAimingEffect()
{
	if (UAbilitySystemComponent* HostASC = GetHostAbilitySystemComponent())
	{
		FGameplayEffectContextHandle ContextHandle = HostASC->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = HostASC->MakeOutgoingSpec(AimingEffect, 1.0f, ContextHandle);
		FActiveGameplayEffectHandle ActiveGameplayEffect = HostASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UZodiacHeroAbility::AdvanceComboIndex()
{
	if (++ComboIndex >= ComboSockets.Num())
	{
		ComboIndex = 0;
	}
}

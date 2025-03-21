// the.quiet.string@gmail.com

#include "AbilitySystem/Hero/ZodiacHeroAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHeroAbilitySystemComponent.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/ZodiacAbilityCost.h"
#include "AbilitySystem/Host/ZodiacHostAbilitySystemComponent.h"
#include "Hero/ZodiacHeroAbilitySlotActor.h"
#include "Character/ZodiacHeroCharacter.h"
#include "Character/ZodiacHostCharacter.h"
#include "Hero/ZodiacHeroAbilitySlot.h"
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

AZodiacHostCharacter* UZodiacHeroAbility::GetZodiacHostCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AZodiacHostCharacter>(CurrentActorInfo->OwnerActor.Get()) : nullptr);
}

AZodiacPlayerController* UZodiacHeroAbility::GetHostPlayerControllerFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AZodiacPlayerController>(GetZodiacHostCharacterFromActorInfo()->GetController()) : nullptr);
}

AZodiacHeroCharacter* UZodiacHeroAbility::GetHeroActorFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AZodiacHeroCharacter>(GetAvatarActorFromActorInfo()) : nullptr);
}

AZodiacHeroCharacter* UZodiacHeroAbility::GetHeroCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<AZodiacHeroCharacter>(GetAvatarActorFromActorInfo()) : nullptr);
}

UZodiacHeroAbilitySlot* UZodiacHeroAbility::GetAssociatedSlot() const
{
	if (FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
	{
		return Cast<UZodiacHeroAbilitySlot>(Spec->SourceObject.Get());
	}

	return nullptr;
}

void UZodiacHeroAbility::AddSlotAsSourceObject(FGameplayEffectContextHandle& EffectContext)
{
	if (UZodiacHeroAbilitySlot* Slot = GetAssociatedSlot())
	{
		EffectContext.AddSourceObject(Slot);
	}
}

FGameplayEffectContextHandle UZodiacHeroAbility::AddSourceSlotToEffectContext(FGameplayEffectContextHandle EffectContext)
{
	AddSlotAsSourceObject(EffectContext);
	return EffectContext;
}

const FGameplayTagContainer* UZodiacHeroAbility::GetCooldownTags() const
{
	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&TempCooldownTags);
	MutableTags->Reset();
	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();
	if (ParentTags)
	{
		MutableTags->AppendTags(*ParentTags);
	}
	MutableTags->AppendTags(CooldownTags);
	
	return MutableTags;
}

bool UZodiacHeroAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	bool bBlocked = false;
	bool bMissing = false;
	
	if (UAbilitySystemComponent* HostASC = GetHostAbilitySystemComponent())
	{
		// Check to see the required/blocked tags of host for this ability
		if (ActivationBlockedTagsHost.Num() || ActivationRequiredTagsHost.Num())
		{
			static FGameplayTagContainer AbilitySystemComponentTags;
			AbilitySystemComponentTags.Reset();

			HostASC->GetOwnedGameplayTags(AbilitySystemComponentTags);

			if (AbilitySystemComponentTags.HasAny(ActivationBlockedTagsHost))
			{
				bBlocked = true;
			}

			if (!AbilitySystemComponentTags.HasAll(ActivationRequiredTagsHost))
			{
				bMissing = true;
			}

			if (bBlocked)
			{
				if (OptionalRelevantTags && BlockedTag.IsValid())
				{
					OptionalRelevantTags->AddTag(BlockedTag);
				}
				return false;
			}
			
			if (bMissing)
			{
				if (OptionalRelevantTags && MissingTag.IsValid())
				{
					OptionalRelevantTags->AddTag(MissingTag);
				}
				return false;
			}
		}
	}

	if (AllowedAimYaw > 0.f)
	{
		if (!IsInAllowedAimRange())
		{
			OptionalRelevantTags->AddTag(ZodiacGameplayTags::Ability_ActivateFail_OutOfAllowedAimRange);
			return false;
		}
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
		HostASC->AddLooseGameplayTags(ActivationOwnedTagsHost);
	}

	UAbilitySystemComponent* Comp = ActorInfo->AbilitySystemComponent.Get();

	// Block and cancel host abilities too.
	Comp->ApplyAbilityBlockAndCancelTags(GetAssetTags(), nullptr, true, BlockAbilitiesWithTag_Host, true, CancelAbilitiesWithTag_Host);
}

bool UZodiacHeroAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    FGameplayTagContainer* OptionalRelevantTags) const
{
	bHasCheckedInitialCost = bHasAppliedInitialCost;
	
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
	{
		return false;
	}
	
	// Verify we can afford any additional costs
	for (const TObjectPtr<UZodiacAbilityCost>& AdditionalCost : AdditionalCosts)
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

void UZodiacHeroAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo) const
{
	bHasCheckedInitialCost = true;
	
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

	//Pay any additional costs
	bool bAbilityHitTarget = false;
	bool bHasDeterminedIfAbilityHitTarget = false;
	for (const TObjectPtr<UZodiacAbilityCost>& AdditionalCost : AdditionalCosts)
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

			if (!ComboToIgnoreAdditionalCost.Contains(ComboIndex))
			{
				AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
			}
		}
	}

	bHasAppliedInitialCost = true;
}

void UZodiacHeroAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		SpecHandle.Data.Get()->DynamicGrantedTags.AppendTags(CooldownTags);
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(ZodiacGameplayTags::SetByCaller_Cooldown, CooldownDuration.GetValueAtLevel(GetAbilityLevel()));
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}

void UZodiacHeroAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Cache whether we were still active before calling the parent
	bool bWasActiveBefore = bIsActive;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (bWasActiveBefore && !bIsActive)
	{
		if (UAbilitySystemComponent* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
		{
			if (IsBlockingOtherAbilities())
			{
				AbilitySystemComponent->ApplyAbilityBlockAndCancelTags(GetAssetTags(), nullptr, false, BlockAbilitiesWithTag_Host, false, CancelAbilitiesWithTag_Host);
			}
		}
	}
	
	if (UAbilitySystemComponent* HostASC = GetHostAbilitySystemComponent())
	{
		HostASC->RemoveLooseGameplayTags(ActivationOwnedTagsHost);
	}

	bHasCheckedInitialCost = false;
	bHasAppliedInitialCost = false;
}

FVector UZodiacHeroAbility::GetWeaponLocation() const
{
	if (USkeletalMeshComponent* MeshComponent =  GetOwningComponentFromActorInfo())
	{
		if (AZodiacHeroAbilitySlotActor* SocketSourceActor = GetCurrentSocketSourceActor())
		{
			FVector Location;
			FRotator Rotator;
			MeshComponent->GetSocketWorldLocationAndRotation(SocketSourceActor->SocketName, Location, Rotator);
			return Location;
		}

		return MeshComponent->GetSocketLocation(FName());
	}

	return  FVector();
}

bool UZodiacHeroAbility::IsInAllowedAimRange() const
{
	if (AllowedAimYaw > 0.f)
	{
		if (AZodiacHeroCharacter* Hero = GetHeroActorFromActorInfo())
		{
			return FMath::Abs(Hero->GetAimYaw()) <= AllowedAimYaw;
		}	
	}

	return true;
}

void UZodiacHeroAbility::ApplySlotReticle()
{
	if (UZodiacHeroAbilitySlot* Slot = GetAssociatedSlot())
	{
		Slot->ChangeReticle();
	}
}

void UZodiacHeroAbility::ClearSlotReticle()
{
	if (UZodiacHeroAbilitySlot* Slot = GetAssociatedSlot())
	{
		Slot->ClearReticle();
	}
}

void UZodiacHeroAbility::ClearCameraModeAfterDuration(float Duration, FTimerHandle& RemoveHandle)
{
	if (UZodiacHeroAbilitySystemComponent* HeroASC = Cast<UZodiacHeroAbilitySystemComponent>(GetHeroAbilitySystemComponentFromActorInfo()))
	{
		HeroASC->ClearCameraModeAfterDuration(CurrentSpecHandle, Duration, RemoveHandle);
	}
}

AZodiacHeroAbilitySlotActor* UZodiacHeroAbility::GetCurrentSocketSourceActor() const
{
	if (AZodiacHeroAbilitySlotActor* const* ActorPtr = CachedSocketSourceActors.Find(ComboIndex))
	{
		if (AZodiacHeroAbilitySlotActor* Actor = *ActorPtr)
		{
			return Actor;
		}
	}
	
	if (UZodiacHeroAbilitySlot* Slot = GetAssociatedSlot())
	{
		TArray<AZodiacHeroAbilitySlotActor*> SpawnedActors = Slot->GetSpawnedActors();
		if (SocketSourceClasses.IsValidIndex(ComboIndex))
		{
			TSubclassOf<AZodiacHeroAbilitySlotActor> ComboActorClass = SocketSourceClasses[ComboIndex];
			for (auto& SpawnedActor : SpawnedActors)
			{
				if (SpawnedActor->IsA(ComboActorClass))
				{
					CachedSocketSourceActors.Add(ComboIndex, SpawnedActor);
					return SpawnedActor;
				}
			}
		}
	}

	return nullptr;
}

void UZodiacHeroAbility::AdvanceCombo()
{
	if (++ComboIndex >= SocketSourceClasses.Num())
	{
		ComboIndex = 0;
	}
}

void UZodiacHeroAbility::ChargeUltimate()
{
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(ChargeUltimateEffectClass, GetAbilityLevel());
	EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(ZodiacGameplayTags::SetByCaller_Ultimate, UltimateChargeAmount.GetValueAtLevel(GetAbilityLevel()));
	
	ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle);
}

void UZodiacHeroAbility::ExecuteGameplayCueOnHost(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParams)
{
	if (UAbilitySystemComponent* HostASC = GetHostAbilitySystemComponent())
	{
		HostASC->ExecuteGameplayCue(GameplayCueTag, GameplayCueParams);
	}
}

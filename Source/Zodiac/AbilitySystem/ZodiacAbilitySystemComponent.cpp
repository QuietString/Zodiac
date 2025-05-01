// Copyright Epic Games, Inc. All Rights Reserved.
// the.quiet.string@gmail.com


#include "ZodiacAbilitySystemComponent.h"

#include "ZodiacGlobalAbilitySystem.h"
#include "ZodiacLogChannels.h"
#include "Abilities/ZodiacGameplayAbility.h"
#include "System/ZodiacAssetManager.h"
#include "System/ZodiacGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAbilitySystemComponent)

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked");

UZodiacAbilitySystemComponent::UZodiacAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();

	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void UZodiacAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UZodiacGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UZodiacGlobalAbilitySystem>(GetWorld()))
	{
		GlobalAbilitySystem->UnregisterASC(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UZodiacAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);
	
	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	
	if (bHasNewPawnAvatar)
	{
		// Register with the global system once we actually have a pawn avatar. We wait until this time since some globally-applied effects may require an avatar.
		if (UZodiacGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UZodiacGlobalAbilitySystem>(GetWorld()))
		{
			GlobalAbilitySystem->RegisterASC(this);
		}
		
		TryActivateAbilitiesOnSpawn();
	}
}

void UZodiacAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc,
                                                          bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		UZodiacGameplayAbility* ZodiacAbilityCDO = Cast<UZodiacGameplayAbility>(AbilitySpec.Ability);
		if (!ZodiacAbilityCDO)
		{
			UE_LOG(LogZodiacAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Non-ZodiacGameplayAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
			continue;
		}

		PRAGMA_DISABLE_DEPRECATION_WARNINGS
				ensureMsgf(AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced, TEXT("CancelAbilitiesByFunc: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
			
				// Cancel all the spawned instances.
				TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
		for (UGameplayAbility* AbilityInstance : Instances)
		{
			UZodiacGameplayAbility* ZodiacAbilityInstance = CastChecked<UZodiacGameplayAbility>(AbilityInstance);

			if (ShouldCancelFunc(ZodiacAbilityInstance, AbilitySpec.Handle))
			{
				if (ZodiacAbilityInstance->CanBeCanceled())
				{
					ZodiacAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), ZodiacAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
				}
				else
				{
					UE_LOG(LogZodiacAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *ZodiacAbilityInstance->GetName());
				}
			}
		}
	}
}

void UZodiacAbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](const UZodiacGameplayAbility* ZodiacAbility, FGameplayAbilitySpecHandle Handle)
	{
		const EZodiacAbilityActivationPolicy ActivationPolicy = ZodiacAbility->GetActivationPolicy();
		return ((ActivationPolicy == EZodiacAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy == EZodiacAbilityActivationPolicy::WhileInputActive));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UZodiacAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UZodiacAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void UZodiacAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	// Process all abilities that activate when the input is held.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UZodiacGameplayAbility* ZodiacAbilityCDO = CastChecked<UZodiacGameplayAbility>(AbilitySpec->Ability);
				
				if (ZodiacAbilityCDO->GetActivationPolicy() == EZodiacAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	// Process all abilities that had their input pressed this frame.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const UZodiacGameplayAbility* ZodiacAbilityCDO = CastChecked<UZodiacGameplayAbility>(AbilitySpec->Ability);

					if (ZodiacAbilityCDO->GetActivationPolicy() == EZodiacAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	if (ActorInfo->AvatarActor->GetLocalRole() == ROLE_SimulatedProxy && ActorInfo->OwnerActor->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ActorInfo->AvatarActor->SetRole(ROLE_AutonomousProxy);
	}
	
	//
	// Try to activate all the abilities that are from presses and holds.
	// We do it all at once so that held inputs don't activate the ability
	// and then also send a input event to the ability because of the press.
	//
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		bool Activated = TryActivateAbility(AbilitySpecHandle);
	}

	// Process all abilities that had their input released this frame.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	// Clear the cached ability handles.
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UZodiacAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

bool UZodiacAbilitySystemComponent::IsActivationGroupBlocked(EZodiacAbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case EZodiacAbilityActivationGroup::Independent:
		// Independent abilities are never blocked.
			bBlocked = false;
		break;

	case EZodiacAbilityActivationGroup::Exclusive_Replaceable:
	case EZodiacAbilityActivationGroup::Exclusive_Blocking:
		// Exclusive abilities can activate if nothing is blocking.
		bBlocked = (ActivationGroupCounts[(uint8)EZodiacAbilityActivationGroup::Exclusive_Blocking] > 0);
		break;

	default:
		checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	return bBlocked;
}

void UZodiacAbilitySystemComponent::AddAbilityToActivationGroup(EZodiacAbilityActivationGroup Group,
	UZodiacGameplayAbility* ZodiacAbility)
{
	check(ZodiacAbility);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);

	ActivationGroupCounts[(uint8)Group]++;

	const bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case EZodiacAbilityActivationGroup::Independent:
		// Independent abilities do not cancel any other abilities.
			break;

	case EZodiacAbilityActivationGroup::Exclusive_Replaceable:
	case EZodiacAbilityActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(EZodiacAbilityActivationGroup::Exclusive_Replaceable, ZodiacAbility, bReplicateCancelAbility);
		break;

	default:
		checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	const int32 ExclusiveCount = ActivationGroupCounts[(uint8)EZodiacAbilityActivationGroup::Exclusive_Replaceable] + ActivationGroupCounts[(uint8)EZodiacAbilityActivationGroup::Exclusive_Blocking];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogZodiacAbilitySystem, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

void UZodiacAbilitySystemComponent::RemoveAbilityFromActivationGroup(EZodiacAbilityActivationGroup Group,
	UZodiacGameplayAbility* ZodiacAbility)
{
	check(ZodiacAbility);
	check(ActivationGroupCounts[(uint8)Group] > 0);

	ActivationGroupCounts[(uint8)Group]--;
}

void UZodiacAbilitySystemComponent::CancelActivationGroupAbilities(EZodiacAbilityActivationGroup Group,
	UZodiacGameplayAbility* IgnoreZodiacAbility, bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreZodiacAbility](const UZodiacGameplayAbility* ZodiacAbility, FGameplayAbilitySpecHandle Handle)
	{
		return ((ZodiacAbility->GetActivationGroup() == Group) && (ZodiacAbility != IgnoreZodiacAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UZodiacAbilitySystemComponent::AddDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
	const TSubclassOf<UGameplayEffect> DynamicTagGE = UZodiacAssetManager::GetSubclass(UZodiacGameData::Get().DynamicTagGameplayEffect);
	if (!DynamicTagGE)
	{
		UE_LOG(LogZodiacAbilitySystem, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to find DynamicTagGameplayEffect [%s]."), *UZodiacGameData::Get().DynamicTagGameplayEffect.GetAssetName());
		return;
	}
	
	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(DynamicTagGE, 1.0f, MakeEffectContext());
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	
	if (!Spec)
	{
		UE_LOG(LogZodiacAbilitySystem, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to make outgoing spec for [%s]."), *GetNameSafe(DynamicTagGE));
		return;
	}
	
	Spec->DynamicGrantedTags.AddTag(Tag);
	
	ApplyGameplayEffectSpecToSelf(*Spec);
}

void UZodiacAbilitySystemComponent::RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag)
{
	const TSubclassOf<UGameplayEffect> DynamicTagGE = UZodiacAssetManager::GetSubclass(UZodiacGameData::Get().DynamicTagGameplayEffect);
	if (!DynamicTagGE)
	{
		UE_LOG(LogZodiacAbilitySystem, Warning, TEXT("RemoveDynamicTagGameplayEffect: Unable to find gameplay effect [%s]."), *UZodiacGameData::Get().DynamicTagGameplayEffect.GetAssetName());
		return;
	}
	
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag));
	Query.EffectDefinition = DynamicTagGE;
	
	RemoveActiveEffects(Query);
}

void UZodiacAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle,
                                                         const FGameplayAbilityActivationInfo& ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}

bool UZodiacAbilitySystemComponent::SetActiveGameplayEffectDuration(FActiveGameplayEffectHandle Handle, float InDuration)
{
	if (!Handle.IsValid())
	{
		return false;
	}

	const FActiveGameplayEffect* ActiveGameplayEffect = GetActiveGameplayEffect(Handle);
	if (!ActiveGameplayEffect)
	{
		return false;
	}

	FActiveGameplayEffect* ActiveEffect = const_cast<FActiveGameplayEffect*>(ActiveGameplayEffect);
	if (InDuration > 0)
	{
		ActiveEffect->Spec.Duration = InDuration;
	}
	else
	{
		ActiveEffect->Spec.Duration = 0.01f;
	}

	ActiveEffect->StartServerWorldTime = ActiveGameplayEffects.GetServerWorldTime();
	ActiveEffect->CachedStartServerWorldTime = ActiveEffect->StartServerWorldTime;
	ActiveEffect->StartWorldTime = ActiveGameplayEffects.GetWorldTime();
	ActiveGameplayEffects.MarkItemDirty(*ActiveEffect);
	ActiveGameplayEffects.CheckDuration(Handle);

	ActiveEffect->EventSet.OnTimeChanged.Broadcast(ActiveEffect->Handle, ActiveEffect->StartWorldTime, ActiveEffect->GetDuration());
	OnGameplayEffectDurationChange(*ActiveEffect);

	return true;
}

bool UZodiacAbilitySystemComponent::IsLocallyPredicted() const
{
	return ScopedPredictionKey.IsValidKey();
}

#if WITH_EDITOR
int32 UZodiacAbilitySystemComponent::HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
	int32 Result = Super::HandleGameplayEvent(EventTag, Payload);

	if (ZodiacConsoleVariables::CVarLogPredictionKey.GetValueOnAnyThread())
	{
		UE_LOG(LogZodiacAbilitySystem, Log, TEXT("%s, %s event prediction key: %d"), GetOwnerActor()->HasAuthority() ? TEXT("Server") : TEXT("Client"), *EventTag.ToString(), ScopedPredictionKey.Current);
	}
	
	return Result;
}
#endif

void UZodiacAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		const UZodiacGameplayAbility* ZodiacAbilityCDO = CastChecked<UZodiacGameplayAbility>(AbilitySpec.Ability);
		ZodiacAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
	}
}

void UZodiacAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.
	if (Spec.IsActive())
	{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
	}
}

void UZodiacAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.
	if (Spec.IsActive())
	{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
	}
}

void UZodiacAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle,
                                                           UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (UZodiacGameplayAbility* ZodiacAbility = CastChecked<UZodiacGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(ZodiacAbility->GetActivationGroup(), ZodiacAbility);
	}
}

void UZodiacAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability,
	bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (UZodiacGameplayAbility* ZodiacAbility = Cast<UZodiacGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(ZodiacAbility->GetActivationGroup(), ZodiacAbility);	
	}
}

void UZodiacAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability,
	const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	if (APawn* Avatar = Cast<APawn>(GetAvatarActor()))
	{
		if (!Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			ClientNotifyAbilityFailed(Ability, FailureReason);
			return;
		}
	}

	HandleAbilityFailed(Ability, FailureReason);
}

void UZodiacAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability,
																			 const FGameplayTagContainer& FailureReason)
{
	HandleAbilityFailed(Ability, FailureReason);
}

void UZodiacAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability,
                                                        const FGameplayTagContainer& FailureReason)
{
#if !UE_BUILD_SHIPPING
	if (ZodiacConsoleVariables::CVarLogAbilityActivation.GetValueOnAnyThread())
	{
		UE_LOG(LogZodiacAbilitySystem, Warning, TEXT("Ability %s failed to activate (tags: %s)"), *GetPathNameSafe(Ability), *FailureReason.ToString());
	}
#endif

	if (const UZodiacGameplayAbility* ZodiacAbility = Cast<const UZodiacGameplayAbility>(Ability))
	{
		ZodiacAbility->OnAbilityFailedToActivate(FailureReason);
	}	
}

void UZodiacAbilitySystemComponent::AddLooseGameplayTagForDuration(FGameplayTag Tag, float Duration, FTimerHandle& RemoveHandle)
{
	AddLooseGameplayTag(Tag);
	
	GetWorld()->GetTimerManager().SetTimer(
		RemoveHandle,
		[ASC = this, TagToRemove = Tag]()
		{
			ASC->RemoveLooseGameplayTag(TagToRemove);
		},
		Duration,
		false
	);
}
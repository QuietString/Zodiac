// the.quiet.string@gmail.com

#include "ZodiacAbilityTask_NotifyPayload.h"

#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAbilityTask_NotifyPayload)

void UZodiacAbilityTask_NotifyPayload::Activate()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (ASC && Ability)
	{
		DelegateHandle = ASC->AbilityReplicatedEventDelegate(EventType, GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(this, &UZodiacAbilityTask_NotifyPayload::OnPayloadCallback);

		if (IsPredictingClient())
		{
			FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());
            
			FPredictionKey CurrentPredictionKey = ASC->ScopedPredictionKey;
			if (ensure(CurrentPredictionKey.IsValidKey()))
			{
				FVector_NetQuantize100 NetPayload(Payload);
				ASC->ServerSetReplicatedEventWithPayload(EventType, GetAbilitySpecHandle(), GetActivationPredictionKey(), CurrentPredictionKey, NetPayload);
				ASC->InvokeReplicatedEventWithPayload(EventType, GetAbilitySpecHandle(), GetActivationPredictionKey(), CurrentPredictionKey, NetPayload);
			}
		}
		else
		{
			if (IsForRemoteClient())
			{
				if (!ASC->CallReplicatedEventDelegateIfSet(EventType, GetAbilitySpecHandle(), GetActivationPredictionKey()))
				{
					SetWaitingOnRemotePlayerData();
				}
			}
		}
	}
	else
	{
		EndTask();
	}
}

UZodiacAbilityTask_NotifyPayload* UZodiacAbilityTask_NotifyPayload::NotifyPayload(UGameplayAbility* OwningAbility, const EAbilityGenericReplicatedEvent::Type EventType, FVector InPayload)
{
	UZodiacAbilityTask_NotifyPayload* Task = NewAbilityTask<UZodiacAbilityTask_NotifyPayload>(OwningAbility);
	Task->EventType = EventType;
	Task->Payload = InPayload;
	return Task;
}

void UZodiacAbilityTask_NotifyPayload::OnPayloadCallback()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC || !Ability)
	{
		EndTask();
		return;
	}

	// Remove delegate to avoid multiple triggers
	ASC->AbilityReplicatedEventDelegate(EventType, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandle);

	FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());

	if (IsPredictingClient())
	{
		// We've sent a predicted event to the server and now we're confirming it
		ASC->ConsumeGenericReplicatedEvent(EventType, GetAbilitySpecHandle(), GetActivationPredictionKey());
	}
	else
	{
		// Server side also consume the event
		ASC->ConsumeGenericReplicatedEvent(EventType, GetAbilitySpecHandle(), GetActivationPredictionKey());
	}
	
	// We are done. Kill us so we don't keep getting broadcast messages
	FAbilityReplicatedData Data = ASC->GetReplicatedDataOfGenericReplicatedEvent(EventType, GetAbilitySpecHandle(), GetActivationPredictionKey());
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnPayload.Broadcast(Data.VectorPayload);
	}

	EndTask();
}


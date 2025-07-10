// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "ZodiacAbilityTask_NotifyPayload.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPayloadDelegate, FVector, PayloadVector);

/**
 * Sends payload vector to server and trigger callback event with maintaining prediction key.
 */
UCLASS()
class ZODIAC_API UZodiacAbilityTask_NotifyPayload : public UAbilityTask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FPayloadDelegate OnPayload;

	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UZodiacAbilityTask_NotifyPayload* NotifyPayload(UGameplayAbility* OwningAbility, EAbilityGenericReplicatedEvent::Type EventType, FVector InPayload);

protected:
	void OnPayloadCallback();
	
	EAbilityGenericReplicatedEvent::Type EventType;
	FVector Payload;
	FDelegateHandle DelegateHandle;
};

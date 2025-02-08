// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacGameplayAbility.h"
#include "ZodiacGameplayAbility_TraversalAction.generated.h"

/**
 * Ability for traversal action, working with UZodiacTraversalComponent.
 * Can be activated by player input or gameplay event.
 */
UCLASS(Abstract)
class ZODIAC_API UZodiacGameplayAbility_TraversalAction : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:
	UZodiacGameplayAbility_TraversalAction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
protected:
	UFUNCTION()
	void OnTraversalFinished();
};

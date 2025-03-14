// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Hero/ZodiacHeroAbility.h"
#include "Traversal/ZodiacTraversalTypes.h"
#include "ZodiacGameplayAbility_Traversal.generated.h"

/**
 * Ability for traversal action, working with UZodiacTraversalComponent.
 * Can be activated by player input or gameplay event.
 */
UCLASS(Abstract)
class ZODIAC_API UZodiacGameplayAbility_Traversal : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:
	UZodiacGameplayAbility_Traversal(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	
protected:
	UFUNCTION()
	void OnTraversalFinished();

	mutable FZodiacTraversalCheckResult TraversalCheckResult;
};

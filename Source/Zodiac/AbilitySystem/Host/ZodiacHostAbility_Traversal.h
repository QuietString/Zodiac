// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHostAbility.h"
#include "Traversal/ZodiacTraversalTypes.h"
#include "ZodiacHostAbility_Traversal.generated.h"


UCLASS(abstract)
class ZODIAC_API UZodiacHostAbility_Traversal : public UZodiacHostAbility
{
	GENERATED_BODY()

public:
	UZodiacHostAbility_Traversal(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	
protected:
	UFUNCTION()
	void OnTraversalFinished();

	mutable FZodiacTraversalCheckResult TraversalCheckResult;
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacGameplayAbility.h"
#include "ZodiacGameplayAbility_Jump.generated.h"

UCLASS(Abstract)
class ZODIAC_API UZodiacGameplayAbility_Jump : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:
	UZodiacGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnDelayFinished();
	
protected:
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	void CharacterJumpStart();

	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	void CharacterJumpStop();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DelayTime = 0.5f;
};

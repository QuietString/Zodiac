// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHeroAbility.h"
#include "ZodiacHeroAbility_Jump.generated.h"

UCLASS(Abstract)
class ZODIAC_API UZodiacHeroAbility_Jump : public UZodiacHeroAbility
{
	GENERATED_BODY()

public:
	UZodiacHeroAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnJustLanded(FGameplayEventData Payload);
	
	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	void CharacterJumpStart();

	UFUNCTION(BlueprintCallable, Category = "Zodiac|Ability")
	void CharacterJumpStop();
};

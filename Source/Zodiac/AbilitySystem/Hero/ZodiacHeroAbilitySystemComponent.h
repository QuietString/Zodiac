// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacHeroAbilitySystemComponent.generated.h"


class UZodiacHostAbilitySystemComponent;

UCLASS()
class ZODIAC_API UZodiacHeroAbilitySystemComponent : public UZodiacAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UZodiacHostAbilitySystemComponent* GetHostAbilitySystemComponent() const { return HostAbilitySystemComponent; }
	void SetHostAbilitySystemComponent(UZodiacHostAbilitySystemComponent* InASC);

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
	
	virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;

	// When should call ApplyAbilityBlockAndCancelTags from HostASC, don't call HostASC's one. 
	virtual void ApplyAbilityBlockAndCancelTags_FromHostASC(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags);

	// Custom latent action call that can execute after the calling ability ends.
	// Cosmetic usage only.
	UFUNCTION(BlueprintCallable)
	void ClearCameraModeAfterDuration(const FGameplayAbilitySpecHandle& OwningSpecHandle, float Duration, FTimerHandle& RemoveHandle);
	
protected:
	UPROPERTY()
	TObjectPtr<UZodiacHostAbilitySystemComponent> HostAbilitySystemComponent;
};

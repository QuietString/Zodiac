// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputSubsystems.h"
#include "ZodiacGameplayAbility.h"
#include "ZodiacGameplayAbility_Sprint.generated.h"

class UInputMappingContext;
class UAbilityTask_WaitInputRelease;
/**
 * 
 */
UCLASS(Abstract)
class ZODIAC_API UZodiacGameplayAbility_Sprint : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:
	UZodiacGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem(AZodiacHostCharacter* ZodiacCharacter);

protected:
	UFUNCTION()
	void OnInputRelease(float TimeHeld);

	void Sprint();
	void Walk();

protected:
	UPROPERTY(EditDefaultsOnly)
	UInputMappingContext* SprintIMC;
	
	UPROPERTY(EditDefaultsOnly)
	float SprintSpeed = 1000.0f;

private:
	UPROPERTY()
	UAbilityTask_WaitInputRelease* WaitInputRelease;
	
	float WalkingSpeed;
};

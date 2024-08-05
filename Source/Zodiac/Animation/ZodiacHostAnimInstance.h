// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility_Sprint.h"
#include "Animation/AnimInstance.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "Character/ZodiacHostCharacter.h"
#include "ZodiacHostAnimInstance.generated.h"

class UZodiacCharacterMovementComponent;

UCLASS()
class ZODIAC_API UZodiacHostAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	void InitializeWithAbilitySystem(UAbilitySystemComponent* InASC);
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
	void UpdateVelocityData();
	
	bool GetIsAiming() const { return bIsAiming; }

protected:
	UFUNCTION(BlueprintCallable)
	void UpdateGait();

public:
	UPROPERTY(BlueprintReadOnly, meta=(DisplayName = CharacterMovement))
	TObjectPtr<UZodiacCharacterMovementComponent> ZodiacCharMovComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FTransform RootTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AimYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float AimPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacGait> Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EZodiacGait> Gait_LastFrame;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AZodiacHostCharacter> HostCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAiming;
};

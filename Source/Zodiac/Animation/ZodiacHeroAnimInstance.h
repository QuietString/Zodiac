// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility_Sprint.h"
#include "Animation/AnimInstance.h"
#include "ZodiacHeroAnimInstance.generated.h"

class UCharacterMovementComponent;
class AZodiacHostCharacter;

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacHeroAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	void InitializeWithAbilitySystem(UAbilitySystemComponent* InASC);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAimingReleaseMontage();
	
protected:
	void UpdateRotationData(float DeltaSeconds, AActor* OwningActor);
	void UpdateAimingData(AZodiacHostCharacter* HostCharacter);
	
	AZodiacHostCharacter* GetHostCharacter() const;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aiming_Data)
	float AimYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aiming_Data)
	float AimPitch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aiming_Data)
	FVector2D RootYawOffsetAngleClamp = FVector2D(-120.f, 100.f);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAiming;
};
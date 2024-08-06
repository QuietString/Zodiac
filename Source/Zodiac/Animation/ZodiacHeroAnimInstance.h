// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility_Sprint.h"
#include "Animation/AnimInstance.h"
#include "ZodiacHeroAnimInstance.generated.h"

class UZodiacHostAnimInstance;
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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable)
	AZodiacHostCharacter* GetHostCharacter() const;

	UFUNCTION(BlueprintCallable)
	UZodiacHostAnimInstance* GetHostAnimInstance() const;

	void OnAimingChanged(bool bHasActivated);
	void OnIsPistolReadyChanged(bool InIsReady);
	
	UFUNCTION(BlueprintImplementableEvent)
	void PlayHideOrRevealGunsMontage(bool bReveal);
	
protected:
	void UpdateRotationData(float DeltaSeconds, AActor* OwningActor);
	void UpdateAimingData(float DeltaSeconds);
	void UpdateBlendData(float DeltaSeconds);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blends")
	float PistolScale;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blends")
	float PistolBlendAlpha;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aiming_Data)
	float AimYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aiming_Data)
	float AimPitch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aiming_Data)
	FVector2D RootYawOffsetAngleClamp = FVector2D(-120.f, 100.f);

	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAiming = false;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	bool bIsGunsHidden = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsHostMoving;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsPistolReady = false;
	
private:
	UPROPERTY()
	TObjectPtr<AZodiacHostCharacter> HostCharacter;
	
	UPROPERTY()
	TObjectPtr<UZodiacHostAnimInstance> HostAnimInstance;
};
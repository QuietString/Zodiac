// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimInstance.h"
#include "Character/ZodiacCharacter.h"
#include "ZodiacHeroAnimInstance.generated.h"

class UAbilitySystemComponent;
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
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	AZodiacCharacter* GetParentCharacter() const;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	UZodiacHostAnimInstance* GetParentAnimInstance() const;

	void OnAimingChanged(bool bHasActivated);
	void OnIsWeaponReadyChanged(bool InIsReady);
	void OnStatusChanged(FGameplayTag Tag, bool bActive);
	
	UFUNCTION(BlueprintImplementableEvent)
	void PlayHideOrRevealGunsMontage(bool bReveal);
	
protected:
	void UpdateMovementData();
	void UpdateRotationData(float DeltaSeconds, AActor* OwningActor);
	void UpdateAimingData(float DeltaSeconds);
	void UpdateBlendData(float DeltaSeconds);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blends")
	float PistolScale;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blends")
	float PistolBlendAlpha;
	
	UPROPERTY(BlueprintReadOnly, Category = Aiming_Data)
	float AimYaw;

	UPROPERTY(BlueprintReadOnly, Category = Aiming_Data)
	float AimPitch;

	UPROPERTY(EditDefaultsOnly, Category = Aiming_Data)
	FVector2D AimYawClampRange = FVector2D(-60.0f, 60.0f);
	
	UPROPERTY(EditDefaultsOnly, Category = Aiming_Data)
	float AimYawBlendSpeed = 5.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Aiming_Data)
	FVector2D RootYawOffsetAngleClamp = FVector2D(-120.f, 100.f);
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsADS;

	UPROPERTY(BlueprintReadOnly)
	bool bIsFocus;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDead;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsTraversal;

	UPROPERTY(BlueprintReadOnly)
	bool bIsMoving;

	UPROPERTY(BlueprintReadOnly)
	bool bIsWeaponReady = false;

	UPROPERTY(BlueprintReadWrite)
	bool bAnimNotify_RevealLeftPistol;

	UPROPERTY(BlueprintReadWrite)
	bool bAnimNotify_RevealRightPistol;
	
	UPROPERTY(BlueprintReadOnly)
	bool bShouldReveal_LeftPistol;

	UPROPERTY(BlueprintReadOnly)
	bool bShouldReveal_RightPistol;

	UPROPERTY(BlueprintReadWrite)
	bool bShouldRaise_RightArm;

	UPROPERTY(BlueprintReadOnly)
	bool bShouldRaise_LeftArm;
	
	UPROPERTY(BlueprintReadOnly)
	bool bApplyAimOffSet;

	// 0 to hide right pistol, 1 to reveal right pistol 
	UPROPERTY(BlueprintReadOnly)
	float RightPistolScaleAlpha;

	UPROPERTY(EditDefaultsOnly)
	float RightPistolAlphaSpeedMultiplier = 5.0f;

	UPROPERTY(BlueprintReadOnly)
	float LeftPistolScaleAlpha;

	UPROPERTY(EditDefaultsOnly)
	float LeftPistolAlphaSpeedMultiplier = 5.0f;
	
private:
	UPROPERTY()
	TObjectPtr<AZodiacCharacter> ParentCharacter;
	
	UPROPERTY()
	TObjectPtr<UZodiacHostAnimInstance> ParentAnimInstance;
};
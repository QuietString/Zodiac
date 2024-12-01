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
	
	void OnStatusChanged(FGameplayTag Tag, bool bHasTag);

protected:
	void UpdateMovementData();
	void UpdateAimingData(float DeltaSeconds);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Blends")
	float RootYawOffset;
	
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
	bool bIsFlying = false;
	
private:
	UPROPERTY()
	TObjectPtr<AZodiacCharacter> ParentCharacter;
	
	UPROPERTY()
	TObjectPtr<UZodiacHostAnimInstance> ParentAnimInstance;
};
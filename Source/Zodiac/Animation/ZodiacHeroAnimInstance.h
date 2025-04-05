// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/ZodiacGameplayEffectTypes.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility_Sprint.h"
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
	
	void InitializeWithAbilitySystem(UAbilitySystemComponent* InASC);
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	AZodiacCharacter* GetParentCharacter() const;

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	UZodiacHostAnimInstance* GetParentAnimInstance() const;
	
	void OnStatusChanged(FGameplayTag Tag, bool bHasTag);

	bool GetIsFocus() const { return bIsFocus; }
	bool GetIsADS() const { return bIsADS; }

protected:
	void UpdateMovementData();
	void UpdateRotationData();
	void UpdateAimingData();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMapReverse GameplayTagPropertyReverseMap;
	
	UPROPERTY(BlueprintReadOnly, Category = "Blends")
	float RootYawOffset;

	UPROPERTY(BlueprintReadOnly, Category = Aiming_Data)
	float AimYaw;

	UPROPERTY(BlueprintReadOnly, Category = Aiming_Data)
	float AimPitch;

	// Root yaw amount to make the rotation of this mesh match with parent mesh.
	UPROPERTY(BlueprintReadOnly, Category = Rotation_Data)
	FRotator RootRotationOffset;

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

	UPROPERTY(BlueprintReadOnly, Transient)
	bool bIsStrafing;
	
	UPROPERTY(BlueprintReadOnly)
	EZodiacExtendedMovementMode ExtendedMovementMode;

	UPROPERTY(BlueprintReadOnly, Transient)
	float MovementAngle;
	
private:
	UPROPERTY(Transient)
	TObjectPtr<AZodiacCharacter> ParentCharacter;
	
	UPROPERTY(Transient)
	TObjectPtr<UZodiacHostAnimInstance> ParentAnimInstance;
};
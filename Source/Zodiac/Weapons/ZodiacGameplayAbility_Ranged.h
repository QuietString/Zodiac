// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ZodiacGameplayAbility.h"
#include "ZodiacGameplayAbility_Ranged.generated.h"

/** Defines where an ability starts its trace from and where it should face */
UENUM(BlueprintType)
enum class EZodiacAbilityTargetingSource : uint8
{
	// From the player's camera towards camera focus
	CameraTowardsFocus,
	// From the pawn's center, in the pawn's orientation
	PawnForward,
	// From the pawn's center, oriented towards camera focus
	PawnTowardsFocus,
	// From the weapon's muzzle or location, in the pawn's orientation
	WeaponForward,
	// From the weapon's muzzle or location, towards camera focus
	WeaponTowardsFocus,
	// Custom blueprint-specified source location
	Custom
};

UCLASS()
class ZODIAC_API UZodiacGameplayAbility_Ranged : public UZodiacGameplayAbility
{
	GENERATED_BODY()

public:

	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	struct FRangedWeaponFiringInput
	{
		// Start of the trace
		FVector StartTrace;

		// End of the trace if aim were perfect
		FVector EndAim;

		// The direction of the trace if aim were perfect
		FVector AimDir;

		// Can we play bullet FX for hits during this trace
		bool bCanPlayBulletFX = false;

		FRangedWeaponFiringInput()
			: StartTrace(ForceInitToZero)
			, EndAim(ForceInitToZero)
			, AimDir(ForceInitToZero)
		{
		}
	};
	
	void StartRangedWeaponTargeting();

	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

	void OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);
	
	void PerformLocalTargeting(OUT TArray<FHitResult>& OutHits);

	FVector GetWeaponTargetingSourceLocation() const;
	FTransform GetTargetingTransform(APawn* SourcePawn, EZodiacAbilityTargetingSource Source) const;

	void PlayAbilityMontage();

	UFUNCTION()
	void OnMontageEnd();
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ForceUnits="Hz"), Category = "Zodiac|Weapon")
	float RateOfFire;

	UPROPERTY(EditDefaultsOnly, meta=(ForceUnits=s), Category = "Zodiac|Weapon")
	float FireInterval;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Damage")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Cues")
	FGameplayTag GameplayCueTag_Firing;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Cues")
	FGameplayTag GameplayCueTag_Impact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Animation")
	UAnimMontage* MontageToPlay;

private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	// make A inverse of B
	void InvertValue(float& A, float& B);
#endif

};

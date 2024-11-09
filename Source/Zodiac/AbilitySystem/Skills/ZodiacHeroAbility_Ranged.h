// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Skills/ZodiacHeroAbility.h"
#include "ZodiacHeroAbility_Ranged.generated.h"

class UZodiacHeroAbilitySlot_RangedWeapon;
/** Defines where an ability starts its trace from and where it should face */
UENUM(BlueprintType)
enum class EZodiacAbilityAimTraceRule : uint8
{
	CameraTowardsFocus			UMETA(ToolTip = "From the player's camera towards camera focus"),
	PawnForward					UMETA(ToolTip = "From the pawn's center, in the pawn's orientation"),
	PawnTowardsFocus			UMETA(ToolTip = "From the weapon's muzzle or location, in the pawn's orientation"),
	WeaponForward				UMETA(ToolTip = "From the weapon's muzzle or location, in the pawn's orientation"),
	WeaponTowardsFocus			UMETA(ToolTip = "From the weapon's muzzle or location, towards camera focus"),
	WeaponTowardsFocusHit	UMETA(ToolTip = "From the weapon's muzzle or location, towards focus trace hit"),
	Custom						UMETA(ToolTip = "Custom blueprint-specified source location")
};

UCLASS()
class ZODIAC_API UZodiacHeroAbility_Ranged : public UZodiacHeroAbility
{
	GENERATED_BODY()

public:
	UZodiacHeroAbility_Ranged(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	struct FRangedAbilityTraceData
	{
		// Start of the trace
		FVector StartTrace;

		// End of the trace if aim were perfect
		FVector EndAim;

		// The direction of the trace if aim were perfect
		FVector AimDir;

		// The weapon instance / source of weapon data
		UZodiacHeroAbilitySlot_RangedWeapon* WeaponData = nullptr;

		// Can we play bullet FX for hits during this trace
		bool bCanPlayBulletFX = false;

		FRangedAbilityTraceData()
			: StartTrace(ForceInitToZero)
			, EndAim(ForceInitToZero)
			, AimDir(ForceInitToZero)
		{
		}
	};

	static int32 FindFirstPawnHitResult(const TArray<FHitResult>& HitResults);

	// Does a single weapon trace, either sweeping or ray depending on if SweepRadius is above zero
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const;

	virtual void AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const;

	UFUNCTION(BlueprintCallable)
	void StartRangedWeaponTargeting();

	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

	// override it in blueprint to not call this function.
	UFUNCTION(BlueprintNativeEvent)
	void OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);

	void PerformLocalTargeting(OUT TArray<FHitResult>& OutHits);

	// Traces all of the bullets in a single cartridge
	void TraceBulletsInCartridge(const FRangedAbilityTraceData& InputData, OUT TArray<FHitResult>& OutHits);

	// Wrapper around WeaponTrace to handle trying to do a ray trace before falling back to a sweep trace if there were no hits and SweepRadius is above zero 
	FHitResult DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const;
	
	UFUNCTION(BlueprintCallable)
	FTransform GetTargetingTransform(EZodiacAbilityAimTraceRule TraceRule) const;

	UFUNCTION(BlueprintCallable)
	FTransform GetWeaponTargetingTransform() const;
	
	FTransform GetTargetingTransform(APawn* OwningPawn, AActor* SourceActor, EZodiacAbilityAimTraceRule Source) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ForceUnits="Hz"), Category = "Gameplay")
	float RateOfFire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ForceUnits=s), Category = "Gameplay")
	float FireInterval;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	EZodiacAbilityAimTraceRule AimTraceRule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	FScalableFloat DamagePerBullet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX|Cues", meta = (Categories = "GameplayCue"))
	FGameplayTag GameplayCueTag_Firing;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX|Cues", meta = (Categories = "GameplayCue"))
	FGameplayTag GameplayCueTag_Impact;

	UPROPERTY(BlueprintReadOnly)
	FGameplayCueParameters GameplayCueParams_Firing;

	UPROPERTY(BlueprintReadOnly)
	FGameplayCueParameters GameplayCueParams_Impact;

private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	// make A inverse of B
	void InvertValue(float& A, float& B);
#endif

};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Skills/ZodiacSkillAbility.h"
#include "ZodiacSkillAbility_Traced.generated.h"

/** Defines where an ability starts its trace from and where it should face */
UENUM(BlueprintType)
enum class EZodiacAbilityTargetingRule : uint8
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
class ZODIAC_API UZodiacSkillAbility_Traced : public UZodiacSkillAbility
{
	GENERATED_BODY()

public:
	UZodiacSkillAbility_Traced(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	struct FRangedSkillTraceData
	{
		// Start of the trace
		FVector StartTrace;

		// End of the trace if aim were perfect
		FVector EndAim;

		// The direction of the trace if aim were perfect
		FVector AimDir;

		// Can we play bullet FX for hits during this trace
		bool bCanPlayBulletFX = false;

		FRangedSkillTraceData()
			: StartTrace(ForceInitToZero)
			, EndAim(ForceInitToZero)
			, AimDir(ForceInitToZero)
		{
		}
	};
	
	UFUNCTION(BlueprintCallable)
	void StartRangedWeaponTargeting();

	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

	UFUNCTION(BlueprintNativeEvent)
	void OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);
	
	void PerformLocalTargeting(OUT TArray<FHitResult>& OutHits);

	FVector GetTargetingSourceLocation() const;
	
	UFUNCTION(BlueprintCallable)
	FTransform GetTargetingTransform() const;

	UFUNCTION(BlueprintCallable)
	FTransform GetFXTargetingTransform() const;
	
	FTransform GetTargetingTransform(APawn* SourcePawn, EZodiacAbilityTargetingRule Source) const;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ForceUnits="Hz"), Category = "Skill")
	float RateOfFire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ForceUnits=s), Category = "Skill")
	float FireInterval;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	EZodiacAbilityTargetingRule TargetingSourceRule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cues")
	FGameplayTag GameplayCueTag_Firing;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cues")
	FGameplayTag GameplayCueTag_Impact;

	UPROPERTY(BlueprintReadOnly)
	FGameplayCueParameters GCNParameters;
	
private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	// make A inverse of B
	void InvertValue(float& A, float& B);
#endif

};

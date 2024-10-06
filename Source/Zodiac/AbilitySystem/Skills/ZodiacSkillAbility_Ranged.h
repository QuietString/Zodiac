// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Skills/ZodiacHeroAbility.h"
#include "ZodiacSkillAbility_Ranged.generated.h"

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
class ZODIAC_API UZodiacSkillAbility_Ranged : public UZodiacHeroAbility
{
	GENERATED_BODY()

public:
	UZodiacSkillAbility_Ranged(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData) override;
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
	
	UFUNCTION(BlueprintCallable)
	FTransform GetTargetingTransform(EZodiacAbilityAimTraceRule TraceRule) const;

	UFUNCTION(BlueprintCallable)
	FTransform GetWeaponTargetingTransform() const;
	
	FTransform GetTargetingTransform(APawn* OwningPawn, AActor* SourceActor, EZodiacAbilityAimTraceRule Source) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ForceUnits="Hz"), Category = "Skill")
	float RateOfFire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ForceUnits=s), Category = "Skill")
	float FireInterval;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	EZodiacAbilityAimTraceRule AimTraceRule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cues")
	FGameplayTag GameplayCueTag_Firing;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cues")
	FGameplayTag GameplayCueTag_Impact;

	UPROPERTY(BlueprintReadOnly)
	FGameplayCueParameters GCNParameters;

	UPROPERTY(BlueprintReadOnly)
	bool bIsAlreadyFiring;
	
private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	// make A inverse of B
	void InvertValue(float& A, float& B);
#endif

};

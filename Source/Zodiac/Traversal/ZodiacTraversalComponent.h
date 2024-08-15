// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacTraversalTypes.h"
#include "ModularGameplay/Public/Components/PawnComponent.h"
#include "ZodiacTraversalComponent.generated.h"

class UChooserTable;
class UCharacterMovementComponent;

UCLASS()
class ZODIAC_API UZodiacTraversalComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UZodiacTraversalComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void OnRegister() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool TryTraversalActionFromAbility(bool bIsInAir);
	FZodiacTraversalCheckResult TryTraversalAction(bool bIsInAr, bool& bTraversalCheckFailed, bool& bMontageSelectionFailed, FText& FailReason);
	
protected:
	float GetTraversalForwardTraceDistance(bool bIsInAir) const;

	bool CapsuleTrace(const FVector& TraceStart, const FVector& TraceEnd, bool bDrawDebug, FHitResult& OutHit, float CapsuleRadius, float CapsuleHalfHeight);

	bool DetermineTraversalType(FZodiacTraversalCheckResult& CheckResult);

	bool FindMatchingAnimMontage(FZodiacTraversalCheckResult& CheckResult);

	UFUNCTION(BlueprintImplementableEvent)
	UAnimMontage* RunChooser(FZodiacTraversalChooserParams ChooserParams, float& SelectedTime, float& WantedPlayRate);

	UFUNCTION(BlueprintImplementableEvent)
	void PerformTraversalAction(FZodiacTraversalCheckResult CheckResult);

protected:
	UPROPERTY(EditAnywhere)
	FVector2D VaultHeightRange = FVector2D(50.0f, 125.0f);

	UPROPERTY(EditAnywhere)
	FVector2D HurdleHeightRange = FVector2D(50.0f, 125.0f);

	UPROPERTY(EditAnywhere)
	FVector2D MantleHeightRange = FVector2D(50.0f, 275.0f);

	UPROPERTY(EditAnywhere)
	FVector2D AirMantleHeightRange = FVector2D(50.0f, 125.0f);

	UPROPERTY(EditAnywhere)
	FVector2D AirTraversalRange = FVector2D(0.0f, 50.0f);
	
private:
	UPROPERTY(ReplicatedUsing=OnRepTraversalCheckResult)
	FZodiacTraversalCheckResult TraversalCheckResult;

	UFUNCTION()
	void OnRepTraversalCheckResult();
};

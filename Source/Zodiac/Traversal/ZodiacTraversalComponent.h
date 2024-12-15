// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacTraversalTypes.h"
#include "ModularGameplay/Public/Components/PawnComponent.h"
#include "ZodiacTraversalComponent.generated.h"

UCLASS()
class ZODIAC_API UZodiacTraversalComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UZodiacTraversalComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void OnRegister() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~End of UActorComponent interface

	UFUNCTION(BlueprintCallable, meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool CanTraversalAction(FText& FailReason);
	
	UFUNCTION(BlueprintCallable)
	void TryActivateTraversalAbility();
	
	void PerformTraversalActionFromAbility();
	
	// Used only for traversal location visualization
	bool CheckFrontLedge(bool bIsInAir, FZodiacTraversalCheckResult& Result, FText& FailReason, FVector& LastTraceLocation, bool bIsTicked);
	
	FSimpleDelegate OnTraversalFinished;

	UFUNCTION(BlueprintImplementableEvent)
	void DrawLedgeLocation(bool bLedgeFound, FVector Location, FVector Normal);
	
protected:
	float GetTraversalForwardTraceDistance(bool bIsInAir) const;
	bool CapsuleTrace(const FVector& TraceStart, const FVector& TraceEnd, FHitResult& OutHit, float CapsuleRadius, float CapsuleHalfHeight, bool bDrawDebug, float
	                  DebugDuration, bool bIsTicked, const TArray<AActor*>& ActorsToIgnore);
	bool DetermineTraversalType(FZodiacTraversalCheckResult& CheckResult);
	bool FindMatchingAnimMontage(FZodiacTraversalCheckResult& CheckResult);

	UFUNCTION(BlueprintImplementableEvent)
	UAnimMontage* RunChooser(FZodiacTraversalChooserParams ChooserParams, float& SelectedTime, float& WantedPlayRate);

	UFUNCTION(BlueprintImplementableEvent)
	void PerformTraversalAction(FZodiacTraversalCheckResult CheckResult);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Notify Traversal Action Finished"))
	void K2_NotifyTraversalActionFinished();

protected:
	UPROPERTY(EditAnywhere, Category = "Traversal")
	bool bEnableFindLedgeOnTick;

	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector2D VaultHeightRange = FVector2D(50.0f, 125.0f);

	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector2D HurdleHeightRange = FVector2D(50.0f, 125.0f);

	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector2D MantleHeightRange = FVector2D(50.0f, 275.0f);

	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector2D MidAirMantleHeightRange = FVector2D(50.0f, 125.0f);

	UPROPERTY(EditAnywhere, Category = "Traversal")
	FVector2D AirTraversalRange = FVector2D(0.0f, 50.0f);
	
private:
	UPROPERTY(ReplicatedUsing=OnRepTraversalCheckResult)
	FZodiacTraversalCheckResult TraversalCheckResult;
	
	UFUNCTION()
	void OnRepTraversalCheckResult();

	// Used for not to call CanTraversalAction again when activating traversal ability.
	FZodiacTraversalCheckResult CheckResultCached;
	bool bHasCached;
};

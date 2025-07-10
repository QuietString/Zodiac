// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZodiacTraversalTypes.h"
#include "ModularGameplay/Public/Components/PawnComponent.h"
#include "ZodiacTraversalComponent.generated.h"

class UAbilitySystemComponent;

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
	bool CanTraversalAction(FGameplayTag& FailReason, FVector& FrontLedgeLocation, FVector& FrontLedgeNormal, AActor*& BlockingActor);
	
	UFUNCTION(BlueprintCallable)
	void TryActivateTraversalAbility();
	
	void PerformTraversalAction_Local();

	UFUNCTION(Server, Reliable)
	void Server_PerformTraversalAction(FZodiacTraversalCheckResult CheckResult);
	
	// Used only for traversal location visualization
	bool CheckFrontLedge(FZodiacTraversalCheckResult& Result, FGameplayTag& FailReason, FVector& LastTraceLocation, bool bIsTicked, AActor*& BlockingActor);
	
	FSimpleDelegate OnTraversalFinished;

	UFUNCTION(BlueprintImplementableEvent)
	void OnFrontLedgeChecked(bool bLedgeFound, FVector Location, FVector Normal);

	void ClearPerformResult();

	FZodiacTraversalCheckResult GetCachedCheckResult() { return CheckResultCached; };
	void ClearCheckResultCache();
	
protected:
	float GetTraversalForwardTraceDistance(bool bIsInAir) const;
	bool CapsuleTrace(const FVector& TraceStart, const FVector& TraceEnd, FHitResult& OutHit, float CapsuleRadius, float CapsuleHalfHeight, bool bDrawDebug, float
	                  DebugDuration, bool bIsTicked, const TArray<AActor*>& ActorsToIgnore, const TArray<UPrimitiveComponent*>& ComponentsToIgnore);
	bool DetermineTraversalType(FZodiacTraversalCheckResult& CheckResult);
	bool FindMatchingAnimMontage(FZodiacTraversalCheckResult& CheckResult);
	
	UFUNCTION(BlueprintImplementableEvent)
	UAnimMontage* RunChooser(FZodiacTraversalChooserParams ChooserParams, float& SelectedTime, float& WantedPlayRate);

	UFUNCTION(BlueprintImplementableEvent)
	void PerformTraversalAction(FZodiacTraversalCheckResult CheckResult);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Notify Traversal Action Finished"))
	void K2_NotifyTraversalActionFinished();

protected:
	UPROPERTY(EditAnywhere, Category = "Traversal|Check")
	bool bEnableFindLedgeOnTick;

	// Distance range where a character can try traversal action.
	UPROPERTY(EditAnywhere, Category = "Traversal|Action")
	FVector2f GroundForwardTraceRange = FVector2f(100.f, 200.f);

	UPROPERTY(EditAnywhere, Category = "Traversal|Action")
	FVector2f AirForwardTraversalRange = FVector2f(50.0f, 200.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Check", meta = (ForceUnits=deg, UIMin = 0, UIMax = 90))
	float AllowedFacingAngle_Moving = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Check", meta = (ForceUnits=deg, UIMin = 0, UIMax = 90))
	float AllowedFacingAngle_Idle = 60.f;

	// Amount of Z offset of capsule trace start location to avoid getting trace hit of a traversal ledge near feet.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traversal|Check", meta = (ClampMin = -100, ClampMax = 100))
	float BottomTraceAvoidance = 10.f;
	
	UPROPERTY(EditAnywhere, Category = "Traversal|Check")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
	
	UPROPERTY(EditAnywhere, Category = "Traversal|Action")
	FVector2D VaultHeightRange = FVector2D(50.0f, 125.0f);

	UPROPERTY(EditAnywhere, Category = "Traversal|Action")
	FVector2D ClimbVaultHeightRange = FVector2D(125.0f, 275.0f);
	
	UPROPERTY(EditAnywhere, Category = "Traversal|Action")
	FVector2D HurdleHeightRange = FVector2D(50.0f, 125.0f);

	UPROPERTY(EditAnywhere, Category = "Traversal|Action")
	FVector2D MantleHeightRange = FVector2D(50.0f, 275.0f);

private:
	UPROPERTY(ReplicatedUsing=OnRepTraversalCheckResult)
	FZodiacTraversalCheckResult TraversalCheckResult;
	
	UFUNCTION()
	void OnRepTraversalCheckResult();

	// Used for not to call CanTraversalAction again when activating traversal ability.
	FZodiacTraversalCheckResult CheckResultCached;
	bool bHasCached;

	bool bIsLocalPredicted = false;

	// Controlled by AllowedFacingAngle_Moving.
	UPROPERTY()
	float DotThreshold_Moving;

	// Controlled by AllowedFacingAngle_Idle.
	UPROPERTY()
	float DotThreshold_Idle;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

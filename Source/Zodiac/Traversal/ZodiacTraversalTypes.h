// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"

#include "ZodiacTraversalTypes.generated.h"

class UPrimitiveComponent;

namespace ZodiacConsoleVariables
{
	static TAutoConsoleVariable<bool> CVarTraversalDrawDebug(
		TEXT("zodiac.Traversal.DrawDebug"),
		false,
		TEXT(""));

	static TAutoConsoleVariable<int> CVarTraversalDebugLevel(
		TEXT("zodiac.Traversal.DebugLevel"),
		0,
		TEXT(""));

	static TAutoConsoleVariable<float> CVarTraversalDrawDuration(
		TEXT("zodiac.Traversal.DrawDuration"),
		3,
		TEXT(""));
}

UENUM(BlueprintType, DisplayName="Traversal Action Type")
enum class EZodiacTraversalActionType : uint8
{
	None,
	Hurdle,
	Vault,
	Mantle,
};

USTRUCT(BlueprintType, DisplayName="Traversal Check Result", meta = (HasNativeMake = "/Script/Zodiac.ZodiacTraversalCheckHelper:MakeTraversalCheckResult", HasNativeBreak = "/Script/Zodiac.ZodiacTraversalCheckHelper:BreakTraversalCheckResult"))
struct FZodiacTraversalCheckResult
{
	GENERATED_BODY()

public:
	FZodiacTraversalCheckResult();
	
	UPROPERTY()
	EZodiacTraversalActionType ActionType;
	
	UPROPERTY()
	bool bHasFrontLedge;
	
	UPROPERTY()
	FVector_NetQuantize FrontLedgeLocation;
	
	UPROPERTY()
	FVector_NetQuantizeNormal FrontLedgeNormal;

	UPROPERTY()
	bool bHasBackLedge;
	
	UPROPERTY()
	FVector_NetQuantize BackLedgeLocation;

	UPROPERTY()
	FVector_NetQuantizeNormal BackLedgeNormal;
	
	UPROPERTY()
	float BackLedgeHeight;

	UPROPERTY()
	bool bHasBackFloor;
	
	UPROPERTY()
	FVector_NetQuantize BackFloorLocation;

	UPROPERTY()
	float ObstacleHeight;

	UPROPERTY()
	float ObstacleDepth;

	UPROPERTY()
	float Speed;
	
	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> HitComponent;
	
	UPROPERTY()
	TWeakObjectPtr<UAnimMontage> ChosenMontage;

	UPROPERTY()
	float StartTime;

	UPROPERTY()
	float PlayRate;
	
public:
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	/** Utility to return the Component that was hit. */
	FORCEINLINE UPrimitiveComponent* GetComponent() const
	{
		return HitComponent.Get();
	}
};

// All members of FZodiacTraversalCheckResult are PODs.
template<> struct TIsPODType<FZodiacTraversalCheckResult> { enum { Value = true }; };

template<>
struct TStructOpsTypeTraits<FZodiacTraversalCheckResult> : public TStructOpsTypeTraitsBase2<FZodiacTraversalCheckResult>
{
	enum
	{
		WithNetSerializer = true,
	};
};

UCLASS()
class UZodiacTraversalCheckHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Traversal", meta = (NativeMakeFunc, AdvancedDisplay="1", FrontLedgeNormal="0,0,1", BackLedgeNormal="0,0,1"))
	static FZodiacTraversalCheckResult MakeTraversalCheckResult(EZodiacTraversalActionType ActionType, bool HasFrontLedge, FVector FrontLedgeLocation, FVector FrontLedgeNormal, bool bHasBackLedge, FVector BackLedgeLocation, FVector BackLedgeNormal, float BackLedgeHeight, bool bHasBackFloor, FVector BackFloorLocation, float ObstacleHeight, float ObstacleDepth, float Speed, UPrimitiveComponent* HitComponent, UAnimMontage* ChosenMontage, float StartTime, float PlayRate);
	
	UFUNCTION(BlueprintPure, Category = "Traversal", meta=(NativeBreakFunc, AdvancedDisplay="1"))
	static void BreakTraversalCheckResult(const struct FZodiacTraversalCheckResult& Check, EZodiacTraversalActionType& ActionType, bool& HasFrontLedge, FVector& FrontLedgeLocation, FVector& FrontLedgeNormal, bool& bHasBackLedge, FVector& BackLedgeLocation, FVector& BackLedgeNormal, float
	                                      & BackLedgeHeight, bool& bHasBackFloor, FVector& BackFloorLocation, float& ObstacleHeight, float& ObstacleDepth, float& Speed, UPrimitiveComponent*& HitComponent, UAnimMontage*& ChosenMontage, float& StartTime, float& PlayRate);
};

USTRUCT(BlueprintType)
struct FZodiacTraversalChooserParams
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EZodiacTraversalActionType ActionType = EZodiacTraversalActionType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleDepth = 0.0f;
};
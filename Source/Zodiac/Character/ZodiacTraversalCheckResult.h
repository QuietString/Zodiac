// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"

#include "ZodiacTraversalCheckResult.generated.h"

class UPrimitiveComponent;

UENUM(BlueprintType, DisplayName="Traversal Action Type")
enum class EZodiacTraversalActionType : uint8
{
	None,
	Hurdle,
	Vault,
	Mantle,
};

USTRUCT(BlueprintType, DisplayName="Traversal Check Result")
struct FZodiacTraversalCheckResult
{
	GENERATED_BODY()

public:
	FZodiacTraversalCheckResult();
	
	UPROPERTY(BlueprintReadWrite)
	EZodiacTraversalActionType ActionType;
	
	UPROPERTY(BlueprintReadWrite)
	bool bHasFrontLedge;

	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantize FrontLedgeLocation;

	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantizeNormal FrontLedgeNormal;

	UPROPERTY(BlueprintReadWrite)
	bool bHasBackLedge;
	
	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantize BackLedgeLocation;

	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantizeNormal BackLedgeNormal;
	
	UPROPERTY(BlueprintReadWrite)
	float BackLedgeHeight;

	UPROPERTY(BlueprintReadWrite)
	bool bHasBackFloor;
	
	UPROPERTY(BlueprintReadWrite)
	FVector_NetQuantize BackFloorLocation;

	UPROPERTY(BlueprintReadWrite)
	float ObstacleHeight;

	UPROPERTY(BlueprintReadWrite)
	float ObstacleDepth;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UPrimitiveComponent> HitComponent;
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UAnimMontage> ChosenMontage;

	UPROPERTY(BlueprintReadWrite)
	float StartTime;

	UPROPERTY(BlueprintReadWrite)
	float PlayRate;
	
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
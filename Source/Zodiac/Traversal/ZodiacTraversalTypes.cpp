// the.quiet.string@gmail.com

#include "ZodiacTraversalTypes.h"
#include "Components/PrimitiveComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacTraversalTypes)

namespace ZodiacConsoleVariables
{
	TAutoConsoleVariable<bool> CVarTraversalDrawDebug(
		TEXT("zodiac.Traversal.ShowDebug"),
		false,
		TEXT(""));

	TAutoConsoleVariable<int> CVarTraversalDebugLevel(
		TEXT("zodiac.Traversal.DebugLevel"),
		5,
		TEXT("1: front ledge\t2: ceiiing\t3: back ledge\t 4: floor"));

	TAutoConsoleVariable<float> CVarTraversalDrawDuration(
		TEXT("zodiac.Traversal.DrawDuration"),
		2.5,
		TEXT(""));
}

FZodiacTraversalCheckResult::FZodiacTraversalCheckResult()
{
	FMemory::Memzero(this, sizeof(FZodiacTraversalCheckResult));
	ActionType = EZodiacTraversalActionType::None;
	bHasFrontLedge = false;
	bHasBackLedge = false;
	bHasBackFloor = false;
}

bool FZodiacTraversalCheckResult::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << ActionType;
	Ar.SerializeBits(&bHasFrontLedge, 1);
	Ar << FrontLedgeLocation;
	Ar << FrontLedgeNormal;
	Ar.SerializeBits(&bHasBackLedge, 1);
	Ar << BackLedgeLocation;
	Ar << BackLedgeNormal;
	Ar << BackLedgeHeight;
	Ar.SerializeBits(&bHasBackFloor, 1);
	Ar << BackFloorLocation;
	Ar << ObstacleHeight;
	Ar << ObstacleDepth;
	Ar << Speed;
	Ar << HitComponent;
	Ar << ChosenMontage;
	Ar << StartTime;
	Ar << PlayRate;
	
	bOutSuccess = true;
	return true;
}

FZodiacTraversalCheckResult UZodiacTraversalCheckHelper::MakeTraversalCheckResult(EZodiacTraversalActionType ActionType, bool HasFrontLedge,
	FVector FrontLedgeLocation, FVector FrontLedgeNormal, bool bHasBackLedge, FVector BackLedgeLocation, FVector BackLedgeNormal,
	float BackLedgeHeight, bool bHasBackFloor, FVector BackFloorLocation, float ObstacleHeight, float ObstacleDepth, float Speed,
	UPrimitiveComponent* HitComponent, UAnimMontage* ChosenMontage, float StartTime, float PlayRate)
{
	FZodiacTraversalCheckResult CheckResult;
	CheckResult.ActionType = ActionType;
	CheckResult.bHasFrontLedge = HasFrontLedge;
	CheckResult.FrontLedgeLocation = FrontLedgeLocation;
	CheckResult.FrontLedgeNormal = FrontLedgeNormal;
	CheckResult.bHasBackLedge = bHasBackLedge;
	CheckResult.BackLedgeLocation = BackLedgeLocation;
	CheckResult.BackLedgeNormal = BackLedgeNormal;
	CheckResult.BackLedgeHeight = BackLedgeHeight;
	CheckResult.bHasBackFloor = bHasBackFloor;
	CheckResult.BackFloorLocation = BackFloorLocation;
	CheckResult.ObstacleHeight = ObstacleHeight;
	CheckResult.ObstacleDepth = ObstacleDepth;
	CheckResult.Speed = Speed;
	CheckResult.HitComponent = HitComponent;
	CheckResult.ChosenMontage = ChosenMontage;
	CheckResult.StartTime = StartTime;
	CheckResult.PlayRate = PlayRate;
	return CheckResult;
}

void UZodiacTraversalCheckHelper::BreakTraversalCheckResult(const FZodiacTraversalCheckResult& Check, EZodiacTraversalActionType& ActionType,
                                                            bool& HasFrontLedge, FVector& FrontLedgeLocation, FVector& FrontLedgeNormal, bool& bHasBackLedge, FVector& BackLedgeLocation,
                                                            FVector& BackLedgeNormal, float& BackLedgeHeight, bool& bHasBackFloor, FVector& BackFloorLocation, float& ObstacleHeight, float& ObstacleDepth,
                                                            float& Speed, UPrimitiveComponent*& HitComponent, UAnimMontage*& ChosenMontage, float& StartTime, float& PlayRate)
{
	ActionType = Check.ActionType;
	HasFrontLedge = Check.bHasFrontLedge;
	FrontLedgeLocation = Check.FrontLedgeLocation;
	FrontLedgeNormal = Check.FrontLedgeNormal;
	bHasBackLedge = Check.bHasBackLedge;
	BackLedgeLocation = Check.BackLedgeLocation;
	BackLedgeNormal = Check.BackLedgeNormal;
	BackLedgeHeight = Check.BackLedgeHeight;
	bHasBackFloor = Check.bHasBackFloor;
	BackFloorLocation = Check.BackFloorLocation;
	ObstacleHeight = Check.ObstacleHeight;
	ObstacleDepth = Check.ObstacleDepth;
	Speed = Check.Speed;
	HitComponent = Check.GetComponent();
	ChosenMontage = Check.ChosenMontage.Get();
	StartTime = Check.StartTime;
	PlayRate = Check.PlayRate;
}

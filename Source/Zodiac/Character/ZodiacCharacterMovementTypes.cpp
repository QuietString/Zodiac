// the.quiet.string@gmail.com


#include "ZodiacTraversalCheckResult.h"
#include "Components/PrimitiveComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacTraversalCheckResult)

FZodiacTraversalCheckResult::FZodiacTraversalCheckResult()
{
	FMemory::Memzero(this, sizeof(FZodiacTraversalCheckResult));
}

bool FZodiacTraversalCheckResult::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << ActionType;
	Ar.SerializeBits(&bHasFrontLedge, 1);  // Serialize as a bit for efficiency
	Ar << FrontLedgeLocation;
	Ar << FrontLedgeNormal;
	Ar.SerializeBits(&bHasBackLedge, 1);
	Ar << BackLedgeLocation;
	Ar << BackLedgeHeight;
	Ar << BackLedgeNormal;
	Ar.SerializeBits(&bHasBackFloor, 1);
	Ar << BackFloorLocation;
	Ar << ObstacleHeight;
	Ar << ObstacleDepth;
	
	Ar << HitComponent;
	Ar << ChosenMontage;

	Ar << StartTime;
	Ar << PlayRate;

	bOutSuccess = true;
	return true;
}

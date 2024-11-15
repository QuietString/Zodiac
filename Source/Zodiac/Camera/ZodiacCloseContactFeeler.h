// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"

#include "ZodiacCloseContactFeeler.generated.h"


/**
 * Struct defining a feeler ray used for making camera move to center of player character to make aim easier toward close target. 
 */
USTRUCT()
struct FZodiacCloseContactFeeler
{
	GENERATED_BODY()

	/** FRotator describing deviance from main forward ray */
	UPROPERTY(EditAnywhere, Category=PenetrationAvoidanceFeeler)
	FRotator AdjustmentRot;
	
	UPROPERTY(EditAnywhere, Category=CloseContactFeeler)
	float ContactDistance;

	/** extent to use for collision when tracing this feeler */
	UPROPERTY(EditAnywhere, Category=CloseContactFeeler)
	float Extent;

	/** minimum frame interval between traces with this feeler if nothing was hit last frame */
	UPROPERTY(EditAnywhere, Category=CloseContactFeeler)
	int32 TraceInterval;

	/** number of frames since this feeler was used */
	UPROPERTY(transient)
	int32 FramesUntilNextTrace;
	
	FZodiacCloseContactFeeler()
		: ContactDistance(0)
		, Extent(0)
		, TraceInterval(0)
		, FramesUntilNextTrace(0)
	{
	}

	FZodiacCloseContactFeeler(const float& InContactDistance,
									const float& InExtent, 
									const int32& InTraceInterval = 0, 
									const int32& InFramesUntilNextTrace = 0)
		: ContactDistance(InContactDistance)
		, Extent(InExtent)
		, TraceInterval(InTraceInterval)
		, FramesUntilNextTrace(InFramesUntilNextTrace)
	{
	}
};

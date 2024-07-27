// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "ZodiacReplicationGraph.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLyraRepGraph, Display, All);


UCLASS(Transient, Config=Engine)
class UZodiacReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	UZodiacReplicationGraph();
};

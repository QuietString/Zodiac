// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacMessageTypes.generated.h"

USTRUCT(BlueprintType)
struct FSkillDurationMessage
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float Cooldown_Duration = 0;
};

USTRUCT(BlueprintType)
struct FUltimateChargeMessage
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float ChargeAmount = 0;
};
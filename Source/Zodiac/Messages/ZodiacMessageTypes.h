// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacMessageTypes.generated.h"

USTRUCT(BlueprintType)
struct FHeroValueChangedMessage
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APawn> Instigator;
	
	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex;
	
	UPROPERTY(BlueprintReadOnly)
	float OldValue;

	UPROPERTY(BlueprintReadOnly)
	float NewValue;

	UPROPERTY(BlueprintReadOnly)
	float MaxValue;

	UPROPERTY(BlueprintReadOnly)
	float OptionalValue;
};

USTRUCT(BlueprintType)
struct FHeroChangedMessage_SkillSlot
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APawn> Instigator = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FText HeroName = FText();
	
	UPROPERTY(BlueprintReadWrite)
	FSlateBrush Brush;

	UPROPERTY(BlueprintReadOnly)
	bool bIsReady;

	UPROPERTY(BlueprintReadWrite)
	float CurrentValue;

	UPROPERTY(BlueprintReadWrite)
	float MaxValue;

	UPROPERTY(BlueprintReadWrite)
	float OptionalValue;
};

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
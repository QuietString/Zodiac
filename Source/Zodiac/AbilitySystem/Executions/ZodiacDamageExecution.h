// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "ZodiacDamageExecution.generated.h"

class UObject;


/**
 *	Execution used by gameplay effects to apply damage to the health attributes.
 */
UCLASS()
class UZodiacDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	UZodiacDamageExecution();

protected:

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};

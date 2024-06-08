// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZodiacDamagePopStyle.generated.h"

class UNiagaraSystem;

UCLASS()
class ZODIAC_API UZodiacDamagePopStyle : public UDataAsset
{
	GENERATED_BODY()

public:
	//Name of the Niagara Array to set the Damage information
	UPROPERTY(EditDefaultsOnly, Category="DamagePop")
	FName NiagaraArrayName;

	//Niagara System used to display the damages
	UPROPERTY(EditDefaultsOnly, Category="DamagePop")
	TObjectPtr<UNiagaraSystem> TextNiagara;
};

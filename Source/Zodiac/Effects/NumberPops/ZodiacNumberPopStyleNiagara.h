// Copyright Epic Games, Inc. All Rights Reserved.
// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZodiacNumberPopStyleNiagara.generated.h"

class UNiagaraSystem;

UCLASS()
class ZODIAC_API UZodiacNumberPopStyleNiagara : public UDataAsset
{
	GENERATED_BODY()

public:
	
	// Name of the Niagra Array to set the Damage information
	UPROPERTY(EditDefaultsOnly, Category="Number Pop")
	FName NiagaraArrayName;

	// Niagara System used to display the damages
	UPROPERTY(EditDefaultsOnly, Category="Number Pop")
	TObjectPtr<UNiagaraSystem> TextNiagara;
};

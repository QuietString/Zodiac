// the.quiet.string@gmail.com

#pragma once

#include "Engine/DataAsset.h"

#include "ZodiacInputData.generated.h"

class UZodiacAbilitySet;
class UInputMappingContext;
class UZodiacInputConfig;

UCLASS(BlueprintType, Const, meta = (DisplayName = "Zodiac Input Data", ShortToolTip = "Data asset used to define a Pawn."))
class ZODIAC_API UZodiacInputData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	UZodiacInputData(const FObjectInitializer& ObjectInitializer);

public:
	
	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Input")
	TObjectPtr<UZodiacInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Input")
	TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;
};

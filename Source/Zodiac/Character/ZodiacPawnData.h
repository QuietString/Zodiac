// the.quiet.string@gmail.com

#pragma once

#include "Engine/DataAsset.h"

#include "ZodiacPawnData.generated.h"

class UZodiacAbilitySet;
class UInputMappingContext;
class UZodiacInputConfig;

/**
 *	Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(BlueprintType, Const, meta = (DisplayName = "Zodiac Pawn Data", ShortToolTip = "Data asset used to define a Pawn."))
class ZODIAC_API UZodiacPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	UZodiacPawnData(const FObjectInitializer& ObjectInitializer);

public:
	
	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Input")
	TObjectPtr<UZodiacInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Input")
	TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;
};

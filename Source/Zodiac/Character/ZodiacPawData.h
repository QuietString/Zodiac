// the.quiet.string@gmail.com

#pragma once

#include "Engine/DataAsset.h"

#include "ZodiacPawData.generated.h"

class UInputMappingContext;
class UZodiacInputConfig;

/**
 *	Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(BlueprintType, Const, meta = (DisplayName = "Zodaic Pawn Data", ShortToolTip = "Data asset used to define a Pawn."))
class ZODIAC_API UZodiacPawData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	UZodiacPawData(const FObjectInitializer& ObjectInitializer);

public:
	
	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Input")
	TObjectPtr<UZodiacInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Input")
	TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZodiacPawnData.generated.h"

class APawn;
//class UZodiacAbilitySet;
class UZodiacInputConfig;
//class UZodiacAbilityTagRelationshipMapping;
//class UZodiacCameraMode;


/**
 * UZodiacPawnData
 *
 *	Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Zodiac Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class UZodiacPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UZodiacPawnData(const FObjectInitializer& ObjectInitializer);

public:

	// Class to instantiate for this pawn (should usually derive from AZodiacPawn or AZodiacCharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Pawn")
	TSubclassOf<APawn> PawnClass;

	// Ability sets to grant to this pawn's ability system.
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Abilities")
	// TArray<UZodiacAbilitySet*> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Abilities")
	// UZodiacAbilityTagRelationshipMapping* TagRelationshipMapping;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Input")
	UZodiacInputConfig* InputConfig;

	// Default camera mode used by player controlled pawns.
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zodiac|Camera")
	// TSubclassOf<UZodiacCameraMode> DefaultCameraMode;
};

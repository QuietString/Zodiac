// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "ZodiacVerbMessageHelpers.generated.h"

struct FGameplayCueParameters;
struct FZodiacVerbMessage;

class APlayerController;
class APlayerState;
class UObject;
struct FFrame;


UCLASS()
class ZODIAC_API UZodiacVerbMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Zodiac")
	static APlayerState* GetPlayerStateFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Zodiac")
	static APlayerController* GetPlayerControllerFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Zodiac")
	static FGameplayCueParameters VerbMessageToCueParameters(const FZodiacVerbMessage& Message);

	UFUNCTION(BlueprintCallable, Category = "Zodiac")
	static FZodiacVerbMessage CueParametersToVerbMessage(const FGameplayCueParameters& Params);
};

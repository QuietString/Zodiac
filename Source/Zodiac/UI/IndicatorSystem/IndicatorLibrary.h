// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "IndicatorLibrary.generated.h"

class AController;
class UZodiacIndicatorManagerComponent;
class UObject;
struct FFrame;

UCLASS()
class ZODIAC_API UIndicatorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UIndicatorLibrary();
	
	UFUNCTION(BlueprintCallable, Category = Indicator)
	static UZodiacIndicatorManagerComponent* GetIndicatorManagerComponent(AController* Controller);
};

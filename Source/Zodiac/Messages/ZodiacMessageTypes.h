// Copyright Epic Games, Inc. All Rights Reserved.
// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZodiacMessageTypes.generated.h"


USTRUCT(BlueprintType, DisplayName = "Cooldown Message")
struct FZodiacHUDMessage_Cooldown
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, meta = (Categories = "HUD.Type.AbilitySlot"))
	FGameplayTag Slot;
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	float Duration = 0;
};

USTRUCT(BlueprintType, DisplayName = "Input Event Message")
struct FZodiacHUDMessage_InputEvent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, meta = (Categories = "HUD.Type.AbilitySlot"))
	FGameplayTag Slot;
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TEnumAsByte<EInputEvent> InputEvent = EInputEvent::IE_MAX;
};

// Represents a generic message of the form Instigator Verb Target (in Context, with Magnitude)
USTRUCT(BlueprintType)
struct FZodiacVerbMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTag Channel;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<UObject> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	TObjectPtr<UObject> Target = nullptr;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer InstigatorTags;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer TargetTags;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	FGameplayTagContainer ContextTags;

	UPROPERTY(BlueprintReadWrite, Category=Gameplay)
	double Magnitude = 1.0;

	// Returns a debug string representation of this message
	ZODIAC_API FString ToString() const;
};
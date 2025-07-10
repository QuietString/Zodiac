// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/PawnComponent.h"
#include "ZodiacNumberPopComponent.generated.h"

USTRUCT(BlueprintType)
struct FZodiacNumberPopRequest
{
	GENERATED_BODY()

	// The world location to create the number pop at
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Number Pops")
	FVector WorldLocation;

	// Tags related to the source/cause of the number pop (for determining a style)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Number Pops")
	FGameplayTagContainer SourceTags;

	// Tags related to the target of the number pop (for determining a style)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zodiac|Number Pops")
	FGameplayTagContainer TargetTags;

	// The number to display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Number Pops")
	int32 NumberToDisplay = 0;

	// Whether the number is 'critical' or not (@TODO: move to a tag)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Number Pops")
	bool bIsCriticalDamage = false;

	FZodiacNumberPopRequest()
		: WorldLocation(ForceInitToZero)
	{
	}
};

UCLASS(Abstract)
class ZODIAC_API UZodiacNumberPopComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UZodiacNumberPopComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable)
	virtual void AddNumberPop(const FZodiacNumberPopRequest& NewRequest) {}
};

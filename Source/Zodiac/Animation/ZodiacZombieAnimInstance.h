// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHostAnimInstance.h"
#include "ZodiacZombieAnimInstance.generated.h"

UCLASS()
class ZODIAC_API UZodiacZombieAnimInstance : public UZodiacHostAnimInstance
{
	GENERATED_BODY()

public:
	// Don't randomize when Seed is zero.
	UFUNCTION(BlueprintImplementableEvent)
	void SelectAnimsBySeed(uint8 Seed);

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float WalkSpeed;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float MovementSpeedMultiplier;
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZodiacMessageTypes.generated.h"

USTRUCT(BlueprintType, DisplayName = "Cooldown Message")
struct FZodiacCooldownMessage
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

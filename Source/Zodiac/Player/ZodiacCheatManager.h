// the.quiet.string@gmail.com
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "ZodiacCheatManager.generated.h"

#ifndef USING_CHEAT_MANAGER
#define USING_CHEAT_MANAGER (1 && !UE_BUILD_SHIPPING)
#endif

class UZodiacAbilitySystemComponent;
DECLARE_LOG_CATEGORY_EXTERN(LogZodiacCheat, Log, All);

UCLASS(Within = PlayerController, MinimalAPI)
class UZodiacCheatManager : public UCheatManager
{
	GENERATED_BODY()
};

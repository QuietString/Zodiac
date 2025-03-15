#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiac, Log, All);
ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiacExperience, Log, All);
ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiacAbilitySystem, Log, All);
ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiacTeams, Log, All);
ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiacMovement, Log, All);
ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiacTraversal, Log, All);
ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiacSpawner, Log, All);

ZODIAC_API FString GetClientServerContextString(UObject* ContextObject = nullptr);

namespace ZodiacConsoleVariables
{
	inline TAutoConsoleVariable<bool> CVarLogPredictionKey(
		TEXT("zodiac.PredictionKey.ShowDebug"),
		false,
		TEXT(""));

	inline TAutoConsoleVariable<bool> CVarLogAbilityActivation(
		TEXT("zodiac.AbilitySystem.ActivationFail.ShowDebug"),
		false,
		TEXT(""));
}
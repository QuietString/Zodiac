#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"

ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiac, Log, All);
ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiacFramework, Log, All);
ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiacAbilitySystem, Log, All);
ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiacTeams, Log, All);
ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiacMovement, Log, All);
ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiacTraversal, Log, All);
ZODIAC_API DECLARE_LOG_CATEGORY_EXTERN(LogZodiacSpawner, Log, All);

#define UE_LOG_WITH_ROLE(CategoryName, Verbosity, Format, ...)            \
UE_LOG(CategoryName, Verbosity, TEXT("%s") Format, ::ZodiacLog::GetRolePrefix(this), ##__VA_ARGS__)

ZODIAC_API FString GetClientServerContextString(UObject* ContextObject = nullptr);

namespace ZodiacLog
{
	// --- for Actors -------------------------------------------------
	FORCEINLINE const TCHAR* GetRolePrefix(const AActor* Actor)
	{
		if (!Actor)
		{
			return TEXT("Unknown: ");
		}

		if (Actor->HasAuthority())
		{
			return TEXT("Server: ");
		}

		switch (Actor->GetLocalRole())
		{
		case ROLE_AutonomousProxy:	return TEXT("Owning Client: ");
		default:					return TEXT("Remote Client: ");
		}
	}

	// --- for Components --------------------------------------------
	FORCEINLINE const TCHAR* GetRolePrefix(const UActorComponent* Comp)
	{
		if (!Comp)
		{
			return TEXT("Unknown: ");
		}

		if (Comp->GetOwnerRole() == ROLE_Authority)
		{
			return TEXT("Server: ");
		}

		// Components expose GetOwnerRole()
		switch (Comp->GetOwnerRole())
		{
		case ROLE_AutonomousProxy:	return TEXT("Owning Client: ");
		default:					return TEXT("Remote Client: ");
		}
	}

	// --- SFINAE fallback: filter AActor and UActorComponent -----------
	template<
	  typename T,
	  typename = std::enable_if_t<
		  !TIsDerivedFrom<T, AActor>::Value &&
		  !TIsDerivedFrom<T, UActorComponent>::Value>>
FORCEINLINE const TCHAR* GetRolePrefix(const T*)
	{
		return TEXT("Unknown: ");
	}
}

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

	inline TAutoConsoleVariable<bool> CVarLogMotionMatching(
		TEXT("zodiac.MotionMatching.ShowDebug"),
		false,
		TEXT(""));
}
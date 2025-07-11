#include "ZodiacLogChannels.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogZodiac);
DEFINE_LOG_CATEGORY(LogZodiacFramework);
DEFINE_LOG_CATEGORY(LogZodiacAbilitySystem);
DEFINE_LOG_CATEGORY(LogZodiacTeams);
DEFINE_LOG_CATEGORY(LogZodiacMovement);
DEFINE_LOG_CATEGORY(LogZodiacTraversal);
DEFINE_LOG_CATEGORY(LogZodiacSpawner);

FString GetClientServerContextString(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	if (AActor* Actor = Cast<AActor>(ContextObject))
	{
		Role = Actor->GetLocalRole();
	}
	else if (UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client");
	}
	else
	{
#if WITH_EDITOR
	if (GIsEditor)
	{
		extern ENGINE_API FString GPlayInEditorContextString;
		return GPlayInEditorContextString;
	}
#endif
	}

	return TEXT("[]");
}

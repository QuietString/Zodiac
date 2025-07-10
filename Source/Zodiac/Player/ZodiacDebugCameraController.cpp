// the.quiet.string@gmail.com


#include "ZodiacDebugCameraController.h"

#include "ZodiacCheatManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacDebugCameraController)

AZodiacDebugCameraController::AZodiacDebugCameraController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CheatClass = UZodiacCheatManager::StaticClass();
}

void AZodiacDebugCameraController::AddCheats(bool bForce)
{
	// Mirrors ZodiacPlayerController's AddCheats() to avoid the player becoming stuck in the debug camera.
#if USING_CHEAT_MANAGER
	Super::AddCheats(true);
#else
	Super::AddCheats(bForce);
#endif // #else //#if USING_CHEAT_MANAGER
	}

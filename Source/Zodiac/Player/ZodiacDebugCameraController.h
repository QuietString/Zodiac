// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Engine/DebugCameraController.h"
#include "ZodiacDebugCameraController.generated.h"

/**
 *	Used for controlling the debug camera when it is enabled via the cheat manager.
 */
UCLASS()
class AZodiacDebugCameraController : public ADebugCameraController
{
	GENERATED_BODY()

public:
	AZodiacDebugCameraController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void AddCheats(bool bForce) override;
};

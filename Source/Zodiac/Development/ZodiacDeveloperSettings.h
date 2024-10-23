// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ZodiacDeveloperSettings.generated.h"

UENUM()
enum class ECheatExecutionTime
{
	// When the cheat manager is created
	OnCheatManagerCreated,

	// When a pawn is possessed by a player
	OnPlayerPawnPossession
};

USTRUCT()
struct FZodiacCheatToRun
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	ECheatExecutionTime Phase = ECheatExecutionTime::OnPlayerPawnPossession;

	UPROPERTY(EditAnywhere)
	FString Cheat;
};

UCLASS(config=EditorPerProjectUserSettings, MinimalAPI)
class UZodiacDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
};

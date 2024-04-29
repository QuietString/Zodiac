// the.quiet.string@gmail.com


#include "GameModes/ZodiacGameMode.h"

#include "ZodiacGameState.h"
#include "Character/ZodiacPlayerCharacter.h"
#include "Player/ZodiacPlayerController.h"
#include "Player/ZodiacPlayerState.h"
#include "System/ZodiacGameSession.h"
#include "UI/ZodiacHUD.h"

AZodiacGameMode::AZodiacGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = AZodiacGameState::StaticClass();
	GameSessionClass = AZodiacGameSession::StaticClass();
	PlayerControllerClass = AZodiacPlayerController::StaticClass();
	PlayerStateClass = AZodiacPlayerState::StaticClass();
	DefaultPawnClass = AZodiacPlayerCharacter::StaticClass();
	HUDClass = AZodiacHUD::StaticClass();
}

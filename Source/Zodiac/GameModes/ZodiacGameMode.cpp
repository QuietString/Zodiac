// the.quiet.string@gmail.com


#include "GameModes/ZodiacGameMode.h"

#include "ZodiacGameState.h"
#include "Character/ZodiacHostCharacter.h"
#include "Player/ZodiacPlayerController.h"
#include "Player/ZodiacPlayerState.h"
#include "System/ZodiacGameSession.h"
#include "UI/ZodiacHUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacGameMode)

AZodiacGameMode::AZodiacGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = AZodiacGameState::StaticClass();
	GameSessionClass = AZodiacGameSession::StaticClass();
	PlayerControllerClass = AZodiacPlayerController::StaticClass();
	PlayerStateClass = AZodiacPlayerState::StaticClass();
	DefaultPawnClass = AZodiacHostCharacter::StaticClass();
	HUDClass = AZodiacHUD::StaticClass();
}

void AZodiacGameMode::BeginPlay()
{
	Super::BeginPlay();
}

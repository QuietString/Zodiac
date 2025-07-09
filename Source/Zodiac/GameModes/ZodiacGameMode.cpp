// the.quiet.string@gmail.com


#include "GameModes/ZodiacGameMode.h"

#include "ZodiacGameState.h"
#include "ZodiacLogChannels.h"
#include "Character/ZodiacCharacter.h"
#include "Character/ZodiacPawnExtensionComponent.h"
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
	DefaultPawnClass = AZodiacCharacter::StaticClass();
	HUDClass = AZodiacHUD::StaticClass();
}

void AZodiacGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

APawn* AZodiacGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save default player pawns into a map
	SpawnInfo.bDeferConstruction = true;
	
	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APawn* ResultPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
		{
			if (UZodiacPawnExtensionComponent* ExtensionComponent = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(ResultPawn))
			{
				if (AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(ResultPawn))
				{
					if (const UZodiacHeroData* HeroData = ZodiacCharacter->GetPawnData())
					{
						ExtensionComponent->SetPawnData(HeroData);
					}
					else
					{
						UE_LOG(LogZodiac, Error, TEXT("Game mode was unable to set PawnData on the spawned pawn [%s]."), *GetNameSafe(ResultPawn));
					}
				}

				ResultPawn->FinishSpawning(SpawnTransform);
		
				return ResultPawn;
			}
			else
			{
				UE_LOG(LogZodiac, Warning, TEXT("Game mode was unable to find PawnExtensionComponent from Spawned pawn [%s]."), *GetNameSafe(ResultPawn));
			}
		}
		else
		{
			UE_LOG(LogZodiac, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
		}
	}
	else
	{
		UE_LOG(LogZodiac, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
	}
	
	
	return nullptr;
}

bool AZodiacGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return Super::ShouldSpawnAtStartSpot(Player);
}

void AZodiacGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

AActor* AZodiacGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	return Super::ChoosePlayerStart_Implementation(Player);
}

void AZodiacGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

bool AZodiacGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return Super::PlayerCanRestart_Implementation(Player);
}

void AZodiacGameMode::InitGameState()
{
	Super::InitGameState();
}

bool AZodiacGameMode::UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	return Super::UpdatePlayerStartSpot(Player, Portal, OutErrorMessage);
}

void AZodiacGameMode::GenericPlayerInitialization(AController* NewPlayer)
{
	Super::GenericPlayerInitialization(NewPlayer);
}

void AZodiacGameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	Super::FailedToRestartPlayer(NewPlayer);
}

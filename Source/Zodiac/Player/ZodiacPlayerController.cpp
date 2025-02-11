// the.quiet.string@gmail.com


#include "ZodiacPlayerController.h"

#include "AbilitySystemGlobals.h"
#include "ZodiacPlayerState.h"
#include "ZodiacCheatManager.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Camera/ZodiacPlayerCameraManager.h"
#include "Character/ZodiacHostCharacter.h"
#include "Development/ZodiacDeveloperSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacPlayerController)

AZodiacPlayerController::AZodiacPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AZodiacPlayerCameraManager::StaticClass();

#if USING_CHEAT_MANAGER
	CheatClass = UZodiacCheatManager::StaticClass();
#endif
}

AZodiacPlayerState* AZodiacPlayerController::GetZodiacPlayerState() const
{
	return CastChecked<AZodiacPlayerState>(PlayerState);
}

UZodiacAbilitySystemComponent* AZodiacPlayerController::GetZodiacAbilitySystemComponent()
{
	if (AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(GetCharacter()))
	{
		return ZodiacCharacter->GetZodiacAbilitySystemComponent();
	}

	return nullptr;
}

FGenericTeamId AZodiacPlayerController::GetGenericTeamId() const
{
	return GetZodiacPlayerState()->GetGenericTeamId();
}

void AZodiacPlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnableCheats();
}

void AZodiacPlayerController::ServerCheat_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		ClientMessage(ConsoleCommand(Msg));
	}
#endif  
}

bool AZodiacPlayerController::ServerCheat_Validate(const FString& Msg)
{
	return true;
}

void AZodiacPlayerController::ServerCheatAll_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		UE_LOG(LogZodiac, Warning, TEXT("ServerCheatAll: %s"), *Msg);
		
	}
#endif
}

bool AZodiacPlayerController::ServerCheatAll_Validate(const FString& Msg)
{
	return true;
}

void AZodiacPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

#if WITH_SERVER_CODE && WITH_EDITOR
	if (GIsEditor && (InPawn != nullptr) && (GetPawn() == InPawn))
	{
		for (const FZodiacCheatToRun& CheatRow : GetDefault<UZodiacDeveloperSettings>()->CheatsToRun)
		{
			if (CheatRow.Phase == ECheatExecutionTime::OnPlayerPawnPossession)
			{
				ConsoleCommand(CheatRow.Cheat, true);
			}
		}
	}
#endif
}

void AZodiacPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetCharacter()))
	{
		if (IsLocalPlayerController())
		{
			if (UZodiacAbilitySystemComponent* HostASC = HostCharacter->GetHostAbilitySystemComponent())
			{
				HostASC->ProcessAbilityInput(DeltaTime, bGamePaused);
			}
			
			if (UZodiacAbilitySystemComponent* HeroASC = HostCharacter->GetHeroAbilitySystemComponent())
			{
				HeroASC->ProcessAbilityInput(DeltaTime, bGamePaused);
			}
		}
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

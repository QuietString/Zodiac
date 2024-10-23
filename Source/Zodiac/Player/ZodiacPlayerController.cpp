// the.quiet.string@gmail.com


#include "ZodiacPlayerController.h"

#include "ZodiacPlayerState.h"
#include "ZodiacCheatManager.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Camera/ZodiacPlayerCameraManager.h"
#include "Character/ZodiacHostCharacter.h"

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

void AZodiacPlayerController::ServerCheat_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		UE_LOG(LogZodiac, Warning, TEXT("ServerCheat: %s"), *Msg);
		ClientMessage(ConsoleCommand(Msg));
	}
#endif 
}

bool AZodiacPlayerController::ServerCheat_Validate(const FString& Msg)
{
	return true;
}

void AZodiacPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(GetCharacter()))
	{
		if (IsLocalPlayerController())
		{
			if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
			{
				ZodiacASC->ProcessAbilityInput(DeltaTime, bGamePaused);
			}
		}
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

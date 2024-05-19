// the.quiet.string@gmail.com


#include "ZodiacPlayerController.h"

#include "ZodiacPlayerState.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Camera/ZodiacPlayerCameraManager.h"
#include "Character/ZodiacPlayerCharacter.h"

AZodiacPlayerController::AZodiacPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AZodiacPlayerCameraManager::StaticClass();
}

AZodiacPlayerState* AZodiacPlayerController::GetZodiacPlayerState() const
{
	return CastChecked<AZodiacPlayerState>(PlayerState);
}

UZodiacAbilitySystemComponent* AZodiacPlayerController::GetZodiacAbilitySystemComponent() const
{
	if (const AZodiacPlayerCharacter* ZodiacPlayerCharacter = Cast<AZodiacPlayerCharacter>(GetCharacter()))
	{
		return ZodiacPlayerCharacter->GetZodiacAbilitySystemComponent();
	}

	return nullptr;
}

void AZodiacPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
}

void AZodiacPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (AZodiacPlayerCharacter* ZodiacPlayerCharacter = Cast<AZodiacPlayerCharacter>(GetCharacter()))
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

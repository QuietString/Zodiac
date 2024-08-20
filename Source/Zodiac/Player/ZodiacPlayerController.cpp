// the.quiet.string@gmail.com


#include "ZodiacPlayerController.h"

#include "ZodiacPlayerState.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Camera/ZodiacPlayerCameraManager.h"
#include "Character/ZodiacHostCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacPlayerController)

AZodiacPlayerController::AZodiacPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AZodiacPlayerCameraManager::StaticClass();
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

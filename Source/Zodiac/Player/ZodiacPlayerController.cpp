// the.quiet.string@gmail.com


#include "ZodiacPlayerController.h"

#include "ZodiacPlayerState.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Character/ZodiacCharacter.h"

AZodiacPlayerController::AZodiacPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

AZodiacPlayerState* AZodiacPlayerController::GetZodiacPlayerState() const
{
	return CastChecked<AZodiacPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UZodiacAbilitySystemComponent* AZodiacPlayerController::GetZodiacAbilitySystemComponent() const
{
	const AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(GetCharacter());
	return ZodiacCharacter->GetZodiacAbilitySystemComponent();
}

void AZodiacPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AZodiacPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		ZodiacASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

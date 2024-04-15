// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "ZodiacPlayerController.generated.h"

class UZodiacAbilitySystemComponent;
class AZodiacPlayerState;
/**
 * AZodiacPlayerController
 *
 * The base controller class used by this project.
 */
UCLASS(Config = Game, meta = (ShortToolTip = "The base player controller class used by this project."))
class ZODIAC_API AZodiacPlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	AZodiacPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	AZodiacPlayerState* GetZodiacPlayerState() const;
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;

	
	virtual void OnPossess(APawn* InPawn) override;

	//~APlayerController interface
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface
};

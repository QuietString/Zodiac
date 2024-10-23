// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "Teams/ZodiacTeamAgentInterface.h"
#include "ZodiacPlayerController.generated.h"

class UZodiacAbilitySystemComponent;
class AZodiacPlayerState;

/**
 * The base controller class used by this project.
 */
UCLASS(Config = Game, meta = (ShortToolTip = "The base player controller class used by this project."))
class ZODIAC_API AZodiacPlayerController : public ACommonPlayerController, public IZodiacTeamAgentInterface
{
	GENERATED_BODY()

public:
	AZodiacPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	AZodiacPlayerState* GetZodiacPlayerState() const;
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent();

	//~IZodiacTeamAgentInterface interface
	virtual FGenericTeamId GetGenericTeamId() const override;
	//~End of IZodiacTeamAgentInterface interface

	// Run a cheat command on the server.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheat(const FString& Msg);
	
	//~APlayerController interface
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface
};

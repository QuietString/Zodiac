// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "Teams/ZodiacTeamAgentInterface.h"
#include "ZodiacPlayerController.generated.h"

class AZodiacHostCharacter;
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
	UZodiacAbilitySystemComponent* GetHeroAbilitySystemComponent();
	AZodiacHostCharacter* GetHostCharacter() const;

	//~IZodiacTeamAgentInterface interface
	virtual FGenericTeamId GetGenericTeamId() const override;
	//~End of IZodiacTeamAgentInterface interface
	
	// Run a cheat command on the server.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheat(const FString& Msg);

	// Run a cheat command on the server for all players.
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCheatAll(const FString& Msg);

	//~APlayerController interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	//~End of APlayerController interface

	void CheckCrosshairTarget();
	
	// Blueprint function so the UI can easily check if we’re aiming at an enemy
	UFUNCTION(BlueprintPure, Category="Crosshair")
	bool IsCrosshairOnEnemy() const { return bAimingAtEnemy; }

	// If you also want to expose *which* actor is aimed at:
	UFUNCTION(BlueprintPure, Category="Crosshair")
	AActor* GetCurrentAimTarget() const { return CurrentAimTarget.Get(); }

protected:
	UPROPERTY()
	bool bAimingAtEnemy = false;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentAimTarget;

	// The radius for the sphere trace
	UPROPERTY(EditAnywhere, Category="Crosshair")
	float TraceSphereRadius = 5.f;

	// The distance from camera to trace forward
	UPROPERTY(EditAnywhere, Category="Crosshair")
	float TraceDistance = 5000.f;

	UPROPERTY(EditAnywhere, Category="Crosshair")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ModularCharacter.h"
#include "ModularPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "System/GameplayTagStack.h"
#include "ZodiacPlayerState.generated.h"

class UZodiacAbilitySystemComponent;

/** Defines the types of client connected */
UENUM()
enum class EZodiacPlayerConnectionType : uint8
{
	// An active player
	Player = 0,

	// Spectator connected to a running game
	LiveSpectator,

	// Spectating a demo recording offline
	ReplaySpectator,

	// A deactivated player (disconnected)
	InactivePlayer
};

/**
 * AZodiacPlayerState
 *
 * Base player state class used by this project.
 */
UCLASS(Config = Game)
class ZODIAC_API AZodiacPlayerState : public AModularPlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AZodiacPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Zodaic|PlayerState")
	UZodiacAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(Replicated)
	EZodiacPlayerConnectionType MyPlayerConnectionType;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;
};

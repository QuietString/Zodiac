// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "ModularCharacter.h"
#include "ModularPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "System/GameplayTagStack.h"
#include "Teams/ZodiacTeamAgentInterface.h"
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
 * Base player state class used by this project.
 */
UCLASS(Config = Game)
class ZODIAC_API AZodiacPlayerState : public AModularPlayerState, public IAbilitySystemInterface, public IZodiacTeamAgentInterface
{
	GENERATED_BODY()

public:
	AZodiacPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void ClientInitialize(AController* C) override;	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;

	//~IZodiacTeamAgentInterface interface
	virtual FGenericTeamId GetGenericTeamId() const override { return static_cast<uint8>(MyTeam); }
	//~End of IZodiacTeamAgentInterface interface
	
private:

	UPROPERTY(Replicated)
	EZodiacPlayerConnectionType MyPlayerConnectionType;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

	UPROPERTY()
	EZodiacTeam MyTeam = EZodiacTeam::Hero;
};

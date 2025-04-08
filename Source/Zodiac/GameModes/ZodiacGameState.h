// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ModularGameState.h"
#include "ZodiacGameState.generated.h"

struct FZodiacVerbMessage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnToggleDayNight);

/**
 *	The base game state class used by this project.
 */
UCLASS(Config = Game)
class ZODIAC_API AZodiacGameState : public AModularGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Zodiac|GameState", meta = (WorldContext = "WorldContextObject"))
	static AZodiacGameState* GetZodiacGameState(const UObject* WorldContextObject);
	
	// Send a message that all clients will (probably) get
	// (use only for client notifications like eliminations, server join messages, etc... that can handle being lost)
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "Zodiac|GameState")
	void MulticastMessageToClients(const FZodiacVerbMessage Message);

	// Send a message that all clients will be guaranteed to get
	// (use only for client notifications that cannot handle being lost)
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Zodiac|GameState")
	void MulticastReliableMessageToClients(const FZodiacVerbMessage Message);

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnToggleDayNight OnToggleDayNight;
};

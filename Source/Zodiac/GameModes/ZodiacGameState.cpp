// the.quiet.string@gmail.com


#include "ZodiacGameState.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/ZodiacMessageTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacGameState)

void AZodiacGameState::MulticastMessageToClients_Implementation(const FZodiacVerbMessage Message)
{
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Channel, Message);
	}
}

void AZodiacGameState::MulticastReliableMessageToClients_Implementation(const FZodiacVerbMessage Message)
{
	MulticastMessageToClients_Implementation(Message);
}

// the.quiet.string@gmail.com


#include "ZodiacEliminationMessageProcessor.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacMessageTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacEliminationMessageProcessor)

void UZodiacEliminationMessageProcessor::StartListening()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	AddListenerHandle(MessageSubsystem.RegisterListener(ZodiacGameplayTags::Gameplay_Message_Elimination, this, &ThisClass::OnEliminationMessage));
}

void UZodiacEliminationMessageProcessor::OnEliminationMessage(FGameplayTag Channel, const FZodiacVerbMessage& Payload)
{
	
}

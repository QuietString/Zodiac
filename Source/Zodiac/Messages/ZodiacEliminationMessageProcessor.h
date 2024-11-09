// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Messages/GameplayMessageProcessor.h"
#include "ZodiacEliminationMessageProcessor.generated.h"


struct FZodiacVerbMessage;

UCLASS(Abstract)
class ZODIAC_API UZodiacEliminationMessageProcessor : public UGameplayMessageProcessor
{
	GENERATED_BODY()

public:
	virtual void StartListening() override;

private:
	void OnEliminationMessage(FGameplayTag Channel, const FZodiacVerbMessage& Payload);

	
};

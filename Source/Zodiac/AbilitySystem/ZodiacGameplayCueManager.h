// the.quiet.string@gmail.com

#pragma once

#include "GameplayCueManager.h"
#include "ZodiacGameplayCueManager.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()

public:
	UZodiacGameplayCueManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static UZodiacGameplayCueManager* Get();
};

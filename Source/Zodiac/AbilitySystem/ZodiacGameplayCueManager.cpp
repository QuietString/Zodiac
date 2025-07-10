// the.quiet.string@gmail.com


#include "ZodiacGameplayCueManager.h"

#include "AbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacGameplayCueManager)

UZodiacGameplayCueManager::UZodiacGameplayCueManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UZodiacGameplayCueManager* UZodiacGameplayCueManager::Get()
{
	return  Cast<UZodiacGameplayCueManager>(UAbilitySystemGlobals::Get().GetGameplayCueManager());
}

// the.quiet.string@gmail.com


#include "ZodiacAssetManager.h"

#include "ZodiacGameplayTags.h"
#include "AbilitySystemGlobals.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"

UZodiacAssetManager& UZodiacAssetManager::Get()
{
	check(GEngine);

	if (UZodiacAssetManager* Singleton = Cast<UZodiacAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	UE_LOG(LogZodiac, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini. It must be set to ZodiacAssetManager"));

	// Fatal error above prevents this from being called.
	return *NewObject<UZodiacAssetManager>();
}

void UZodiacAssetManager::StartInitialLoading()
{
	// This does all of the scanning, need to do this now even if loads are deferred
	Super::StartInitialLoading();

	InitializeGameplayCueManager();
}

void UZodiacAssetManager::PreBeginPIE(bool bStartSimulate)
{
	Super::PreBeginPIE(bStartSimulate);
}

void UZodiacAssetManager::InitializeGameplayCueManager()
{
	//@TODO: 
}

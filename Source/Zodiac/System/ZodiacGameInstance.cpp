// the.quiet.string@gmail.com


#include "ZodiacGameInstance.h"

#include "ZodiacGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"

void UZodiacGameInstance::Init()
{
	Super::Init();

	// Register our custom init states
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(ZodiacGameplayTags::InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(ZodiacGameplayTags::InitState_DataAvailable, false, ZodiacGameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(ZodiacGameplayTags::InitState_DataInitialized, false, ZodiacGameplayTags::InitState_DataAvailable);

		ComponentManager->RegisterInitState(ZodiacGameplayTags::InitState_Host_HeroSpawned, false, ZodiacGameplayTags::InitState_DataInitialized);
		ComponentManager->RegisterInitState(ZodiacGameplayTags::InitState_Host_HeroDataAvailable, false, ZodiacGameplayTags::InitState_Host_HeroSpawned);
		ComponentManager->RegisterInitState(ZodiacGameplayTags::InitState_Host_HeroDataInitialized, false, ZodiacGameplayTags::InitState_Host_HeroDataAvailable);
		ComponentManager->RegisterInitState(ZodiacGameplayTags::InitState_Host_HeroGameplayReady, false, ZodiacGameplayTags::InitState_Host_HeroDataInitialized);

		ComponentManager->RegisterInitState(ZodiacGameplayTags::InitState_GameplayReady, false, ZodiacGameplayTags::InitState_Host_HeroGameplayReady);
	}
}

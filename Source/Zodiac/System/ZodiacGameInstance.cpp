// the.quiet.string@gmail.com


#include "System/ZodiacGameInstance.h"

#include "CommonSessionSubsystem.h"
#include "CommonUserSubsystem.h"
#include "ZodiacGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Player/ZodiacLocalPlayer.h"


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
		ComponentManager->RegisterInitState(ZodiacGameplayTags::InitState_GameplayReady, false, ZodiacGameplayTags::InitState_DataInitialized);
	}

	// Initialize the debug key with a set value for AES256. This is not secure and for example purposes only.
	// DebugTestEncryptionKey.SetNum(32);
	//
	// for (int32 i = 0; i < DebugTestEncryptionKey.Num(); ++i)
	// {
	// 	DebugTestEncryptionKey[i] = uint8(i);
	// }

	if (UCommonSessionSubsystem* SessionSubsystem = GetSubsystem<UCommonSessionSubsystem>())
	{
		//@TODO: SessionSubsystem->OnPreClientTravelEvent.AddUObject(this, &UZodiacGameInstance::OnPreClientTravelToSession);
	}
}

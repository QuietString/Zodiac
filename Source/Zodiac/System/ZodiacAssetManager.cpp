// the.quiet.string@gmail.com


#include "ZodiacAssetManager.h"

#include "ZodiacLogChannels.h"

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

UObject* UZodiacAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		TUniquePtr<FScopeLogTime> LogTimePtr;

		if (ShouldLogAssetLoads())
		{
			LogTimePtr = MakeUnique<FScopeLogTime>(*FString::Printf(TEXT("Synchronously loaded asset [%s]"), *AssetPath.ToString()), nullptr, FScopeLogTime::ScopeLog_Seconds);
		}

		if (UAssetManager::IsInitialized())
		{
			return UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false);
		}

		// Use LoadObject if asset manager isn't ready yet.
		return AssetPath.TryLoad();
	}

	return nullptr;
}

bool UZodiacAssetManager::ShouldLogAssetLoads()
{
	static bool bLogAssetLoads = FParse::Param(FCommandLine::Get(), TEXT("LogAssetLoads"));
	return bLogAssetLoads;
}

void UZodiacAssetManager::AddLoadedAsset(const UObject* Asset)
{
	if (ensureAlways(Asset))
	{
		FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);
		LoadedAssets.Add(Asset);
	}
}

void UZodiacAssetManager::StartInitialLoading()
{
	// This does all of the scanning, need to do this now even if loads are deferred
	Super::StartInitialLoading();

	// @TODO: STARTUP_JOB(InitializeGameplayCueManager());
}

void UZodiacAssetManager::PreBeginPIE(bool bStartSimulate)
{
	Super::PreBeginPIE(bStartSimulate);
}

void UZodiacAssetManager::InitializeGameplayCueManager()
{
	//@TODO: 
}

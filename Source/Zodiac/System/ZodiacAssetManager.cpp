// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacAssetManager.h"

#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacGameplayCueManager.h"
#include "ZodiacGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAssetManager)

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

const UZodiacGameData& UZodiacAssetManager::GetGameData()
{
	return GetOrLoadTypedGameData<UZodiacGameData>(ZodiacGameDataPath);
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

	InitializeGameplayCueManager();
}

void UZodiacAssetManager::PreBeginPIE(bool bStartSimulate)
{
	Super::PreBeginPIE(bStartSimulate);
}

UPrimaryDataAsset* UZodiacAssetManager::LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass,
	const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType)
{
	UPrimaryDataAsset* Asset = nullptr;

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Loading GameData Object"), STAT_GameData, STATGROUP_LoadTime);
	if (!DataClassPath.IsNull())
	{
#if WITH_EDITOR
		FScopedSlowTask SlowTask(0, FText::Format(NSLOCTEXT("ZodiacEditor", "BeginLoadingGameDataTask", "Loading GameData {0}"), FText::FromName(DataClass->GetFName())));
		const bool bShowCancelButton = false;
		const bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);
#endif
		UE_LOG(LogZodiac, Log, TEXT("Loading GameData: %s ..."), *DataClassPath.ToString());
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("    ... GameData loaded!"), nullptr);

		// This can be called recursively in the editor because it is called on demand from PostLoad so force a sync load for primary asset and async load the rest in that case
		if (GIsEditor)
		{
			Asset = DataClassPath.LoadSynchronous();
			LoadPrimaryAssetsWithType(PrimaryAssetType);
		}
		else
		{
			TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);
			if (Handle.IsValid())
			{
				Handle->WaitUntilComplete(0.0f, false);

				// This should always work
				Asset = Cast<UPrimaryDataAsset>(Handle->GetLoadedAsset());
			}
		}
	}

	if (Asset)
	{
		GameDataMap.Add(DataClass, Asset);
	}
	else
	{
		// It is not acceptable to fail to load any GameData asset. It will result in soft failures that are hard to diagnose.
		UE_LOG(LogZodiac, Fatal, TEXT("Failed to load GameData asset at %s. Type %s. This is not recoverable and likely means you do not have the correct data to run %s."), *DataClassPath.ToString(), *PrimaryAssetType.ToString(), FApp::GetProjectName());
	}

	return Asset;
}

void UZodiacAssetManager::InitializeGameplayCueManager()
{
	SCOPED_BOOT_TIMING("UZodiacAssetManager::InitializeGameplayCueManager");

	UZodiacGameplayCueManager* GCM = UZodiacGameplayCueManager::Get();
	check(GCM);
}

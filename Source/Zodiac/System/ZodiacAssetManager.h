// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ZodiacAssetManager.generated.h"

/**
 * UZodiacAssetManager
 *
 *	Game implementation of the asset manager that overrides functionality and stores game-specific types.
 *	It is expected that most games will want to override AssetManager as it provides a good place for game-specific loading logic.
 *	This class is used by setting 'AssetManagerClassName' in DefaultEngine.ini.
 */
UCLASS(Config = Game)
class ZODIAC_API UZodiacAssetManager : public UAssetManager
{
	GENERATED_BODY()

	// Returns the AssetManger singleton object.
	static UZodiacAssetManager& Get();

protected:

	//~UAssetManager interface
	virtual void StartInitialLoading() override;
#if WITH_EDITOR
	virtual void PreBeginPIE(bool bStartSimulate) override;
#endif
	//~End of UAssetManager interface

private:

	// Sets up the ability system
	void InitializeGameplayCueManager();
};

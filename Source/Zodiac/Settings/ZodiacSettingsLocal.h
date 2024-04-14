// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "ZodiacSettingsLocal.generated.h"

class UZodiacLocalPlayer;
class UPlayerMappableInputConfig;

UCLASS()
class UZodiacSettingsLocal : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UZodiacSettingsLocal();

	static UZodiacSettingsLocal* Get();
	
	//~UObject interface
	virtual void BeginDestroy() override;
	//~End of UObject interface

	//~UGameUserSettings interface
	virtual void LoadSettings(bool bForceReload) override;
	virtual void ResetToCurrentSettings() override;
	virtual void SetToDefaults() override;
	virtual void ApplyNonResolutionSettings() override;
	//~End of UGameUserSettings interface
	
	//////////////////////////
	/// Keybindings
public:

	/** Sets the controller representation to use, a single platform might support multiple kinds of controllers.
	* For example, Win64 games could be played with both an XBox or Playstation controller.
	*/
	UFUNCTION()
	void SetControllerPlatform(const FName InControllerPlatform);
	UFUNCTION()
	FName GetControllerPlatform() const;

private:

	/**
	 * The name of the controller hte player is using. This is maps to the name of a UCommonInputBaseControllerData
	 * that is available on this current platform. The gamepad data are registered per platform, you'll find them
	 * in <Platform>Game.ini files listed under +ControllerData=...
	 */
	UPROPERTY(Config)
	FName ControllerPlatform;

	UPROPERTY(Config)
	FName ControllerPreset = TEXT("Default");

	/** The name of the current input config that the user has selected. */
	UPROPERTY(Config)
	FName InputConfigName = TEXT("Default");

private:
	void ReapplyThingsDueToPossibleDeviceProfileChange();
};

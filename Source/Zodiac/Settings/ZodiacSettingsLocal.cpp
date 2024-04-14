// the.quiet.string@gmail.com


#include "ZodiacSettingsLocal.h"

#include "CommonInputSettings.h"
#include "CommonInputSubsystem.h"
#include "PlayerMappableInputConfig.h"


UZodiacSettingsLocal::UZodiacSettingsLocal()
{
}

UZodiacSettingsLocal* UZodiacSettingsLocal::Get()
{
	return GEngine ? CastChecked<UZodiacSettingsLocal>(GEngine->GetGameUserSettings()) : nullptr;
}

void UZodiacSettingsLocal::BeginDestroy()
{
	// @TODO: OnApplicationActivationStateChanged()
	Super::BeginDestroy();
}

void UZodiacSettingsLocal::LoadSettings(bool bForceReload)
{
	Super::LoadSettings(bForceReload);
	
}

void UZodiacSettingsLocal::ResetToCurrentSettings()
{
	Super::ResetToCurrentSettings();
}

void UZodiacSettingsLocal::SetToDefaults()
{
	Super::SetToDefaults();
}

void UZodiacSettingsLocal::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();

	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetTypedOuter<ULocalPlayer>()))
	{
		InputSubsystem->SetGamepadInputType(ControllerPlatform);
	}
}

void UZodiacSettingsLocal::ReapplyThingsDueToPossibleDeviceProfileChange()
{
	ApplyNonResolutionSettings();
}

void UZodiacSettingsLocal::SetControllerPlatform(const FName InControllerPlatform)
{
	if (ControllerPlatform != InControllerPlatform)
	{
		ControllerPlatform = InControllerPlatform;

		// Apply the change to the common input subsystem so that we refresh any input icons we're using.
		if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetTypedOuter<ULocalPlayer>()))
		{
			InputSubsystem->SetGamepadInputType(ControllerPlatform);
		}
	}
}

FName UZodiacSettingsLocal::GetControllerPlatform() const
{
	return ControllerPlatform;
}

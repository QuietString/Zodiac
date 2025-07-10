// the.quiet.string@gmail.com

#include "ZodiacDeveloperSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacDeveloperSettings)

UZodiacDeveloperSettings::UZodiacDeveloperSettings()
{
}

FName UZodiacDeveloperSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

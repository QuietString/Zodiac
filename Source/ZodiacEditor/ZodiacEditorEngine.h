// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Editor/UnrealEdEngine.h"
#include "ZodiacEditorEngine.generated.h"

/**
 * 
 */
UCLASS()
class ZODIACEDITOR_API UZodiacEditorEngine : public UUnrealEdEngine
{
	GENERATED_BODY()

public:

protected:
	virtual FGameInstancePIEResult PreCreatePIEInstances(const bool bAnyBlueprintErrors, const bool bStartInSpectatorMode, const float PIEStartTime, const bool bSupportsOnlinePIE, int32& InNumOnlinePIEInstances) override;
};

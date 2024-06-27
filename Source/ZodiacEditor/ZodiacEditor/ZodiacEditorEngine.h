// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Editor/UnrealEdEngine.h"
#include "ZodiacEditorEngine.generated.h"


/**
 * 
 */
UCLASS()
class UZodiacEditorEngine : public UUnrealEdEngine
{
	GENERATED_BODY()

public:
	UZodiacEditorEngine(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual void Init(IEngineLoop* InEngineLoop) override;
	virtual void Start() override;
	virtual void Tick(float DeltaSeconds, bool bIdleMode) override;
	
	virtual FGameInstancePIEResult PreCreatePIEInstances(const bool bAnyBlueprintErrors, const bool bStartInSpectatorMode, const float PIEStartTime, const bool bSupportsOnlinePIE, int32& InNumOnlinePIEInstances) override;
};

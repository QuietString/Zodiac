// the.quiet.string@gmail.com


#include "ZodiacEditorEngine.h"

UZodiacEditorEngine::UZodiacEditorEngine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZodiacEditorEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);
}

void UZodiacEditorEngine::Start()
{
	Super::Start();
}

void UZodiacEditorEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
	Super::Tick(DeltaSeconds, bIdleMode);
}

FGameInstancePIEResult UZodiacEditorEngine::PreCreatePIEInstances(const bool bAnyBlueprintErrors,
	const bool bStartInSpectatorMode, const float PIEStartTime, const bool bSupportsOnlinePIE,
	int32& InNumOnlinePIEInstances)
{
	return Super::PreCreatePIEInstances(bAnyBlueprintErrors, bStartInSpectatorMode, PIEStartTime, bSupportsOnlinePIE,
	                                    InNumOnlinePIEInstances);
}

// the.quiet.string@gmail.com


#include "ZodiacEngineCommands.h"

#include "EngineUtils.h"

#define LOCTEXT_NAMESPACE "ZODIACEDITOR" 

void FRefreshActorsUtility::RefreshActors()
{
	UE_LOG(LogTemp, Warning, TEXT("refresh"));
}

void FZodiacEngineCommands::RegisterCommands()
{
	UI_COMMAND(_RefreshAllActors, "Refresh All Actors", "Rerun construction scripts of all actors in a level.", EUserInterfaceActionType::Button, FInputChord());
}

void FZodiacEngineCommands::RefreshAllActors()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("No editor world found."));
		return;
	}
	
	int32 RefreshedActorCount = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor)
		{
			// Re-run the construction script
			Actor->RerunConstructionScripts();
			RefreshedActorCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Refreshed %d actors."), RefreshedActorCount);
}

#undef LOCTEXT_NAMESPACE

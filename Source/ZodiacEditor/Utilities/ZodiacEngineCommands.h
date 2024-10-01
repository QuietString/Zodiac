// the.quiet.string@gmail.com

#pragma once

#include "Framework/Commands/Commands.h"

class FRefreshActorsUtility
{
public:
	static void RefreshActors();
};
/**
 * 
 */
class ZODIACEDITOR_API FZodiacEngineCommands : public TCommands<FZodiacEngineCommands>
{
public:
	FZodiacEngineCommands()
	: TCommands<FZodiacEngineCommands>
	(
		TEXT("ZODIACEDITOR"), // Context name for fast lookup
		NSLOCTEXT("Contexts", "ZodiacEngine", "Zodiac Editor Plugin"), // Localized context name for displaying
		NAME_None, // Parent context name
		NAME_None // Icon Style Set
		)
	{
	}

	// TCommand<> interface
	virtual void RegisterCommands() override;

	static void RefreshAllActors();

	TSharedPtr<FUICommandInfo> _RefreshAllActors;
};

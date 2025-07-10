#pragma once

#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogZodiacEditor, Log, All);

class FZodiacEditorModule : public IModuleInterface
{
	typedef FZodiacEditorModule ThisClass;
	
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void ExtendMenu();

	void AddLevelEditorExtension(FMenuBarBuilder& MenuBarBuilder);

	void AddZodiacMainMenuExtenstion(FMenuBuilder& MenuBuilder);
	
private:
	TSharedPtr<class FUICommandList> ZodiacCommands;
};

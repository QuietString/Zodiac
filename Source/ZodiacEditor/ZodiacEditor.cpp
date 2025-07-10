#include "ZodiacEditor.h"

#include "LevelEditor.h"
#include "UMGEditorModule.h"
#include "Utilities/ZodiacEngineCommands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ZodiacEditor/Utilities/SemiUniformGridSlotExtension.h"

#define LOCTEXT_NAMESPACE "FZodiacEditorModule"

IMPLEMENT_MODULE(FZodiacEditorModule, ZodiacEditor)

DEFINE_LOG_CATEGORY(LogZodiacEditor);

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void FZodiacEditorModule::StartupModule()
{
	// @TODO: use IDesignerExtensionFactory instead?
	IUMGEditorModule& EditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");
	EditorModule.GetDesignerExtensibilityManager().Get()->AddDesignerExtension(MakeShareable(new FSemiUniformGridSlotExtension()));
	
	//ExtendMenu();
}

void FZodiacEditorModule::ShutdownModule()
{
	IUMGEditorModule& EditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");
	TArray<TSharedRef<FDesignerExtension>> Extensions = EditorModule.GetDesignerExtensibilityManager().Get()->GetExternalDesignerExtensions();
	if (Extensions.Num() > 0)
	{
		for (auto& Extension : Extensions)
		{
			EditorModule.GetDesignerExtensibilityManager().Get()->RemoveDesignerExtension(Extension);
		}
	}
}

void FZodiacEditorModule::ExtendMenu()
{
	ZodiacCommands = MakeShareable(new FUICommandList);

	FZodiacEngineCommands::Register();
	const FZodiacEngineCommands& Commands = FZodiacEngineCommands::Get();

	ZodiacCommands->MapAction(
		Commands._RefreshAllActors,
		FExecuteAction::CreateLambda([]() { return FZodiacEngineCommands::RefreshAllActors(); }));

	TSharedRef<FExtender> Extender = MakeShareable(new FExtender);

	Extender->AddMenuBarExtension(
		"Edit",
		EExtensionHook::After,
		ZodiacCommands,
		FMenuBarExtensionDelegate::CreateRaw(this, &FZodiacEditorModule::AddLevelEditorExtension)
		);

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(Extender);
}

void FZodiacEditorModule::AddLevelEditorExtension(FMenuBarBuilder& MenuBarBuilder)
{
	MenuBarBuilder.AddPullDownMenu(
		LOCTEXT("ZodiacLabel", "Zodiac Editor"),
		LOCTEXT("ZodiacMenu_ToolTip", "Run construction scripts"),
		FNewMenuDelegate::CreateRaw(this, &FZodiacEditorModule::AddZodiacMainMenuExtenstion),
		"View");
	
	// MenuBarBuilder.BeginSection("Zodiac Editor", LOCTEXT("ZodiacEditor", "Zodiac Editor"));
	// MenuBarBuilder.AddMenuEntry(FZodiacEngineCommands::Get()._RefreshAllActors);
	//
	// MenuBarBuilder.EndSection();
}

void FZodiacEditorModule::AddZodiacMainMenuExtenstion(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("Rerun", LOCTEXT("ZodiacLabel", "Rerun"));
	MenuBuilder.AddMenuEntry(FZodiacEngineCommands::Get()._RefreshAllActors);

	MenuBuilder.EndSection();
}


PRAGMA_ENABLE_DEPRECATION_WARNINGS
#undef LOCTEXT_NAMESPACE
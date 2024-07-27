#include "ZodiacEditor.h"

#include "UMGEditorModule.h"
#include "ZodiacEditor/Utilities/SemiUniformGridSlotExtension.h"

#define LOCTEXT_NAMESPACE "FZodiacEditorModule"

DEFINE_LOG_CATEGORY(LogZodiacEditor);

class FZodiacEditorModule : public FDefaultGameModuleImpl
{
	typedef FZodiacEditorModule ThisClass;

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	
	virtual void StartupModule() override
	{
		// @TODO: use IDesignerExtensionFactory instead?
		IUMGEditorModule& EditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");
		EditorModule.GetDesignerExtensibilityManager().Get()->AddDesignerExtension(MakeShareable(new FSemiUniformGridSlotExtension()));
	}

	virtual void ShutdownModule() override
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

	PRAGMA_ENABLE_DEPRECATION_WARNINGS
};


#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FZodiacEditorModule, ZodiacEditor)
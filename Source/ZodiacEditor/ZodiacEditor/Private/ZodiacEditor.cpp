#include "ZodiacEditor.h"

#include "UMGEditorModule.h"
#include "ZodiacEditor/ZodiacEditor/Utilities/SemiUniformGridSlotExtension.h"

#define LOCTEXT_NAMESPACE "FZodiacEditorModule"

DEFINE_LOG_CATEGORY(LogZodiacEditor);

class FZodiacEditorModule : public FDefaultGameModuleImpl
{
	typedef FZodiacEditorModule ThisClass;

	virtual void StartupModule() override
	{
		IUMGEditorModule& EditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");
		EditorModule.GetDesignerExtensibilityManager().Get()->AddDesignerExtension(MakeShareable(new FSemiUniformGridSlotExtension));
	}
};


#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FZodiacEditorModule, ZodiacEditor)
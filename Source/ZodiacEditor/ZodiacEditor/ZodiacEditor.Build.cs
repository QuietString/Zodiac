using UnrealBuildTool;

public class ZodiacEditor : ModuleRules
{
    public ZodiacEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "EditorFramework",
                "UnrealEd",
                "Zodiac"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "SlateReflector",
                "UMGEditor",
                "UMG",
                "EditorWidgets"
            }
        );
    }
}
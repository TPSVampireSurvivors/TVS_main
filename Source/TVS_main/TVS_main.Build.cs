// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TVS_main : ModuleRules
{
	public TVS_main(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TVS_main",
			"TVS_main/Variant_Platforming",
			"TVS_main/Variant_Platforming/Animation",
			"TVS_main/Variant_Combat",
			"TVS_main/Variant_Combat/AI",
			"TVS_main/Variant_Combat/Animation",
			"TVS_main/Variant_Combat/Gameplay",
			"TVS_main/Variant_Combat/Interfaces",
			"TVS_main/Variant_Combat/UI",
			"TVS_main/Variant_SideScrolling",
			"TVS_main/Variant_SideScrolling/AI",
			"TVS_main/Variant_SideScrolling/Gameplay",
			"TVS_main/Variant_SideScrolling/Interfaces",
			"TVS_main/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

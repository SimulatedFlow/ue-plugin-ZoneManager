// Copyright 2026 Simulated Flow All Rights Reserved.

using UnrealBuildTool;

public class ZoneManager : ModuleRules
{
	public ZoneManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"UMG",
			"InputCore",
			"Projects",
		});
	}
}

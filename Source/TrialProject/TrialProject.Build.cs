// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TrialProject : ModuleRules
{
	public TrialProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "OnlineSubsystem", "OnlineSubsystemUtils"});
	}
}
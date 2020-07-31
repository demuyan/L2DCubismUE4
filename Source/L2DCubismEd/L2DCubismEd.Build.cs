// Fill out your copyright notice in the Description page of Project Settings.

using System;
using System.IO;
using UnrealBuildTool;

public class L2DCubismEd : ModuleRules
{
	public L2DCubismEd(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;
//        bUseRTTI = true;
//        bEnableExceptions = true;

#if UE_4_24_OR_LATER
        DefaultBuildSettings = BuildSettingsVersion.V1;
#endif

        PublicIncludePaths.AddRange(new string[]{
            Path.Combine(ModuleDirectory, "Public"),
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(ModuleDirectory, "../L2DCubism/Public"),
            Path.Combine(ModuleDirectory, "../L2DCubism/Private"),
            Path.Combine(ModuleDirectory, "../L2DCubism/Private/Render"),
            Path.Combine(ModuleDirectory, "../L2DCubism/SDK/Framework/src"),
            Path.Combine(ModuleDirectory, "../ThirdParty/SDK/Core/include")
        });

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",

            "InputCore",
            "BlueprintGraph",
            "SlateCore",

            "L2DCubism",

        });

	}
}

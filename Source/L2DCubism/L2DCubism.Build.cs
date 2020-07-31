// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

using System;
using System.IO;
using UnrealBuildTool;

public class L2DCubism : ModuleRules
{
	public L2DCubism(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;
//        bUseRTTI = true;
//        bEnableExceptions = true;

        PublicDefinitions.AddRange(new string[]
        {
            "CSM_CORE_WIN32_DLL=0"
        });

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "Engine",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "CoreUObject",
            "Projects",
            "RenderCore",
            "SlateCore",
            "RHI",
            "Projects",
//            "UnrealEd",
        });

        //The path for the header files
        PublicIncludePaths.AddRange(new string[]
        {
            "L2DCubism/Public",
        });

        //The path for the source files
        PrivateIncludePaths.AddRange(new string[]
        {
            "L2DCubism/Private",
            "L2DCubism/Private/Render",
        });

        LoadCubism4CoreLib(Target);
	}

    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string CubismSDKPath
    {
        get { return "SDK"; }
    }

    private string CoreSDKPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../ThirdParty", CubismSDKPath, "Core")); }
    }

    private string CubismFrameworkPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, CubismSDKPath, "Framework")); }
    }

    private bool IsDebugBuild
    {
        get
        {
            switch (Target.Configuration)
            {
                case UnrealTargetConfiguration.DebugGame:
                case UnrealTargetConfiguration.Debug:
                case UnrealTargetConfiguration.Development:
                    return true;
            }

            return false;
        }
    }

    public bool LoadCubism4CoreLib(ReadOnlyTargetRules Target)
    {
        bool IsLibrarySupported = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            IsLibrarySupported = true;

            // Cubism3SDK Core

            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x86_64" : "x86";
            string LibrariesPath = Path.Combine(CoreSDKPath, "lib", "windows", PlatformString, "141");

            Console.WriteLine("... LibrariesPath -> " + LibrariesPath);

            string libFileName = "Live2DCubismCore_MT.lib";
            if (IsDebugBuild)
            {
                libFileName = "Live2DCubismCore_MTd.lib";
            }
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, libFileName));
        }

        if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            IsLibrarySupported = true;

            // Cubism3SDK Core
            string LibrariesPath = Path.Combine(CoreSDKPath, "lib", "macos");

            Console.WriteLine("... LibrariesPath -> " + LibrariesPath);
            string libFileName = "libLive2DCubismCore.a";
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, libFileName));
        }

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            IsLibrarySupported = true;

            // Cubism3SDK Core
            string LibrariesPathArm64 = Path.Combine(CoreSDKPath,  "lib", "android", "arm64-v8a");
            string LibrariesPathArmeabiv7a = Path.Combine(CoreSDKPath,  "lib", "android", "armeabi-v7a");

            Console.WriteLine("... LibrariesPath -> " + LibrariesPathArm64);
            string libFileName = "libLive2DCubismCore.a";
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPathArm64, libFileName));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPathArmeabiv7a, libFileName));
        }

        if (IsLibrarySupported)
        {
            // CubismCore Include path
            string CoreSDKIncludePath = Path.Combine(CoreSDKPath, "include");
            Console.WriteLine("... CoreSDKIncludePath -> " + CoreSDKIncludePath);
            PrivateIncludePaths.Add(CoreSDKIncludePath);

            string FrameworkIncludePath = Path.Combine( CubismFrameworkPath, "src");
            Console.WriteLine("... FrameworkIncludePath -> " + FrameworkIncludePath);
            PrivateIncludePaths.Add(FrameworkIncludePath);
        }

        return IsLibrarySupported;

    }

}

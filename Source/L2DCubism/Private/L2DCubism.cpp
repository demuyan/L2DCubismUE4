// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubism.h"
#include "Core.h"
#include "ShaderCore.h"
#include "Modules/ModuleManager.h"
#include "L2DCubismAsset.h"
#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(LogL2DCubism);

#define LOCTEXT_NAMESPACE "IL2DCubismModule"

void IL2DCubismModule::StartupModule()
{
    UE_LOG(LogL2DCubism, Log, TEXT("L2DCubismModule has started!"));

    FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("L2DCubism"))->GetBaseDir(), TEXT("Source"), TEXT("L2DCubism"), TEXT("Shader"));
    AddShaderSourceDirectoryMapping(TEXT("/Shader"), PluginShaderDir);
}

void IL2DCubismModule::ShutdownModule()
{
    UE_LOG(LogL2DCubism, Log, TEXT("L2DCubismModule has shut down"));
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(IL2DCubismModule, L2DCubism)
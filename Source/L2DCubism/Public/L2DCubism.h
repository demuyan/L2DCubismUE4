// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IL2DCubismModule : public IModuleInterface
{
public:

    static inline IL2DCubismModule& Get() {
        return FModuleManager::LoadModuleChecked< IL2DCubismModule >("L2DCubism");
    }

    static inline bool IsAvailable() {
        return FModuleManager::Get().IsModuleLoaded("L2DCubism");
    }

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

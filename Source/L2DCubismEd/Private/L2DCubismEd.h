// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleManager.h"

class IL2DCubismEdModule : public IModuleInterface
{
public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

public:
    static inline IL2DCubismEdModule& Get() {
        return FModuleManager::LoadModuleChecked< IL2DCubismEdModule >("L2DCubismEd");
    }

    static inline bool IsAvailable() {
        return FModuleManager::Get().IsModuleLoaded("L2DCubismEd");
    }
};


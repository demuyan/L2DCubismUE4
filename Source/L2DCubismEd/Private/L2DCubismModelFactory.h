// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#pragma once

#include "L2DCubismAsset.h"
#include "UnrealEd.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include <ICubismModelSetting.hpp>
#include "L2DCubismModelFactory.generated.h"

// DECLARE_LOG_CATEGORY_EXTERN(LogL2DCubism, Log, All);

/**
 * .model3.jsonƒtƒ@ƒCƒ‹‚ð“Ç‚Þ
 */
UCLASS()
class UL2DCubismModelFactory : public UFactory
{
    GENERATED_UCLASS_BODY()

    UL2DCubismAsset* CubismAsset = nullptr;
    
    FText GetDisplayName() const;
    virtual bool FactoryCanImport(const FString& Filename) override;
    bool DoesSupportClass(UClass* Class);
    UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
                                const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn,
                                bool& bOutOperationCanceled) override;
    void ReadAssets(const FString& LocalPath);
    FString GetName(FString group, size_t i);

    void PreloadMotionGroup(FString group, ICubismModelSetting* ModelSetting, FString ModelHomeDir);
    void GetMotion(FString group, size_t i, FString MotioinFileName);

};
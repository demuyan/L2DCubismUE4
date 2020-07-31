// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#pragma once

#include "UnrealEd.h"
// #include "UObject/ObjectMacros.h"
// #include "Factories/Factory.h"
#include "L2DCubismMoc3Factory.generated.h"

/**
 * .moc3ƒtƒ@ƒCƒ‹‚ð“Ç‚Þ
 */
UCLASS()
class UL2DCubismMoc3Factory : public UFactory
{
    GENERATED_UCLASS_BODY()

    FText GetDisplayName() const;
    virtual bool FactoryCanImport(const FString& Filename) override;
    bool DoesSupportClass(UClass* Class);
    UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
                                const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn,
                                bool& bOutOperationCanceled) override;
};
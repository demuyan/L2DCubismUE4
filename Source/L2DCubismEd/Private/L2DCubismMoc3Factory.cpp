// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubismMoc3Factory.h"
#include "L2DCubismAsset.h"
#include "L2DCubismMoc3.h"

#define LOCTEXT_NAMESPACE "L2DCubismModelFactory"

UL2DCubismMoc3Factory::UL2DCubismMoc3Factory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Formats.Add(TEXT("moc3;Live2D Cubism Moc3"));

    bCreateNew = false;
    bEditAfterNew = true;
    bEditorImport = true;
    bText = false;

    SupportedClass = UL2DCubismMoc3::StaticClass();
    
    ImportPriority = DefaultImportPriority + 2;
}

FText UL2DCubismMoc3Factory::GetDisplayName() const
{
    return LOCTEXT("Live2DCubismFactoryDescription", "Live2D Cubism Moc3");
}

bool UL2DCubismMoc3Factory::FactoryCanImport(const FString& Filename)
{
    return (0 < Filename.Find(TEXT("moc3")));
}

bool UL2DCubismMoc3Factory::DoesSupportClass(UClass * Class)
{
    return (Class == UL2DCubismAsset::StaticClass());
}

UObject* UL2DCubismMoc3Factory::FactoryCreateFile(UClass* InClass,
    UObject* InParent,
    FName InName,
    EObjectFlags Flags,
    const FString& Filename,
    const TCHAR* Parms,
    FFeedbackContext* Warn,
    bool& bOutOperationCanceled)
{
    UL2DCubismMoc3* CubismMoc3 = NewObject<UL2DCubismMoc3>(InParent, InClass, InName, Flags);

    TArray<uint8> RawData;
    FFileHelper::LoadFileToArray(RawData, *Filename);
    CubismMoc3->SetMoc3(RawData);
    return CubismMoc3;
}


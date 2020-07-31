// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubismModelFactory.h"
#include "L2DCubismAsset.h"
#include "L2DCubismMain.h"

#define LOCTEXT_NAMESPACE "L2DCubismModelFactory"

UL2DCubismModelFactory::UL2DCubismModelFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Formats.Add(TEXT("json;Live2D Cubism Model"));

    bCreateNew = false;
    bEditAfterNew = true;
    bEditorImport = true;
    bText = true;

    SupportedClass = UL2DCubismAsset::StaticClass();
    
    ImportPriority = DefaultImportPriority + 1;
}

FText UL2DCubismModelFactory::GetDisplayName() const
{
    return LOCTEXT("Live2DCubismFactoryDescription", "Live2D Cubism ModelInfo");
}

bool UL2DCubismModelFactory::FactoryCanImport(const FString& Filename)
{
    return (0 < Filename.Find(TEXT("model3.json")));
}

bool UL2DCubismModelFactory::DoesSupportClass(UClass * Class)
{
    return (Class == UL2DCubismAsset::StaticClass());
}

UObject* UL2DCubismModelFactory::FactoryCreateFile(UClass* InClass,
    UObject* InParent,
    FName InName,
    EObjectFlags Flags,
    const FString& Filename,
    const TCHAR* Parms,
    FFeedbackContext* Warn,
    bool& bOutOperationCanceled)
{
    CubismAsset = NewObject<UL2DCubismAsset>(InParent, InClass, InName, Flags);
    ReadAssets(Filename);

    return CubismAsset;
}

void UL2DCubismModelFactory::ReadAssets(const FString& LocalPath)
{
    FString OutPath, OutFilename, OutExt;

    FPaths::Split(LocalPath, OutPath, OutFilename, OutExt);

    FString ModelHomeDir = OutPath + "/";
    FString AssetName = OutFilename;
    FString FileName = OutFilename + "." + OutExt;

    FString LocalFullPath = ModelHomeDir + FileName;
//  UE_LOG(LogL2DCubism, Log, TEXT("[APP]load model setting"));
//  FString PathName = CubismAsset->GetPathName();
    
    TArray<uint8> Data;

    if (!FFileHelper::LoadFileToArray(Data, *LocalFullPath))
    {
        return;
    }

    CubismAsset->SetModelSettingJson(Data);
    ICubismModelSetting* ModelSetting = static_cast<ICubismModelSetting*>(CubismAsset->GetModelSettingJson());
//  CubismModelSettingJson* ModelSetting = new CubismModelSettingJson(Data.GetData(), Data.Num());

    TArray<uint8> RawData;
    FString ModelFileName(ModelSetting->GetModelFileName());
    if (0 < ModelFileName.Len())
    {
//      UE_LOG(LogL2DCubism, Log, TEXT("[APP]create model: %s"), *FullPath);

        FFileHelper::LoadFileToArray(RawData, *LocalFullPath);
        CubismAsset->SetModel(RawData);
    }

    if (0 < ModelSetting->GetExpressionCount())
    {
        for (csmInt32 i = 0; i < ModelSetting->GetExpressionCount(); i++)
        {
            //          FString ExpressionName(ModelSetting->GetExpressionName(i));
            FString ExpressionFileName(ModelSetting->GetExpressionFileName(i));
            LocalFullPath = ModelHomeDir + ExpressionFileName;

            FFileHelper::LoadFileToArray(RawData, *LocalFullPath);
            CubismAsset->SetExpression(i, RawData);
//          ModelExpression.Add(Data);
        }
    }

    FString PhysicsFileName(ModelSetting->GetPhysicsFileName());
    if (0 < PhysicsFileName.Len())
    {
        const FString FullName = ModelHomeDir + PhysicsFileName;

        FFileHelper::LoadFileToArray(RawData, *FullName);
        CubismAsset->SetPhysics(RawData);
    }

    const FString PoseFileName(ModelSetting->GetPoseFileName());
    if (0 < PoseFileName.Len())
    {
        LocalFullPath = ModelHomeDir + PoseFileName;
        //      UE_LOG(LogL2DCubism, Log, TEXT("[APP]Pose: %s"), *FullPath);

        FFileHelper::LoadFileToArray(RawData, *LocalFullPath);
        CubismAsset->SetPose(RawData);
    }

    FString UserDataFile(ModelSetting->GetUserDataFile());
    if (0 < PoseFileName.Len())
    {
        LocalFullPath = ModelHomeDir + UserDataFile;
        //      UE_LOG(LogL2DCubism, Log, TEXT("[APP]UserData: %s"), *FullPath);

        FFileHelper::LoadFileToArray(RawData, *LocalFullPath);
        CubismAsset->SetUserData(RawData);
    }

    for (csmInt32 ModelTextureNumber = 0; ModelTextureNumber < ModelSetting->GetTextureCount(); ModelTextureNumber++)
    {
        FString TextureFileName(ModelSetting->GetTextureFileName(ModelTextureNumber));
        if (TextureFileName.Len() <= 0)
        {
            continue;
        }
        CubismAsset->SetTextureFileName(ModelTextureNumber, TextureFileName);
    }

    for (csmInt32 i = 0; i < ModelSetting->GetMotionGroupCount(); i++)
    {
        FString Group(ModelSetting->GetMotionGroupName(i));
        PreloadMotionGroup(Group, ModelSetting, ModelHomeDir);
    }
}

FString UL2DCubismModelFactory::GetName(FString group, size_t i)
{
    return FString::Format(TEXT("{0}_{1}"), { group, i });
}

void UL2DCubismModelFactory::PreloadMotionGroup(FString group, ICubismModelSetting* ModelSetting, FString ModelHomeDir)
{
    for (csmInt32 i = 0; i < ModelSetting->GetMotionCount(TCHAR_TO_ANSI(*group)); ++i)
    {
        const FString MotionFileName(ModelSetting->GetMotionFileName(TCHAR_TO_ANSI(*group), i));
        FString FullPath = ModelHomeDir + MotionFileName;

        FString Name = GetName(group, i);
        GetMotion(group, i, FullPath);
    }
}

void UL2DCubismModelFactory::GetMotion(FString group, size_t i, FString FullPath)
{
    TArray<uint8> Data;

    FFileHelper::LoadFileToArray(Data, *FullPath);
    FString Name = GetName(group, i);

    CubismAsset->SetMotion(Name, Data);
}


// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubismAsset.h"

UL2DCubismAsset::UL2DCubismAsset(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ModelData = TArray<uint8>();
}

void UL2DCubismAsset::BeginDestroy()
{
    Super::BeginDestroy();
}

FString UL2DCubismAsset::GetName()
{
    return AssetName;
}

ICubismModelSetting* UL2DCubismAsset::GetModelSetting()
{
    ParseModelSettingJson();
    return ModelSetting;
}

void UL2DCubismAsset::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);
    if (Ar.IsSaving() || Ar.IsLoading())
    {
        if (0 < ModelExpression.Num())
        {
            Ar << ModelExpression;
        }
        Ar << MotionData;
	}
}

void UL2DCubismAsset::SetTextureFileName(csmInt32 i, const FString& TexttureFileName)
{
    TextureFileNames.Insert(TexttureFileName, i);
}

void UL2DCubismAsset::ParseModelSettingJson()
{
    ModelSetting = new CubismModelSettingJson(ModelSettingJson.GetData(), ModelSettingJson.Num());
}

void UL2DCubismAsset::SetModelSettingJson(TArray<uint8> RawData)
{
    ModelSettingJson = RawData;
    ParseModelSettingJson();
}

CubismModelSettingJson* UL2DCubismAsset::GetModelSettingJson()
{
    return ModelSetting;
}

TArray<uint8> UL2DCubismAsset::GetModel()
{
    return ModelData;
}

void UL2DCubismAsset::SetModel(TArray<uint8> RawData)
{
    ModelData = RawData;
}

void UL2DCubismAsset::SetExpression(int i, TArray<uint8> RawData)
{
    ModelExpression.Insert(RawData, i);
}

TArray<uint8> UL2DCubismAsset::GetExpression(int i)
{
    return ModelExpression[i];
}

void UL2DCubismAsset::SetPhysics(TArray<uint8> RawData)
{
    ModelPhysics = RawData;
}

TArray<uint8> UL2DCubismAsset::GetPhysics()
{
    return ModelPhysics;
}

void UL2DCubismAsset::SetPose(TArray<uint8> RawData)
{
    ModelPose = RawData;
}

TArray<uint8> UL2DCubismAsset::GetPose()
{
    return ModelPose;
}

void UL2DCubismAsset::SetUserData(TArray<uint8> RawData)
{
    ModelUserData = RawData;
}

TArray<uint8> UL2DCubismAsset::GetUserData()
{
    return ModelUserData;
}

void UL2DCubismAsset::SetMotion(FString MotionFileName, TArray<uint8> RawData)
{
    MotionData.Add(MotionFileName, RawData);
}

TArray<uint8> UL2DCubismAsset::GetMotion(FString MotionFileName)
{
    return MotionData[MotionFileName];
}
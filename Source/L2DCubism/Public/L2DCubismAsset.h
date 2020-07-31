// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <CubismFramework.hpp>

#include <ICubismModelSetting.hpp>
#include <Model/CubismUserModel.hpp>
#include <CubismModelSettingJson.hpp>
#include <Model/CubismUserModel.hpp>
#include <ICubismModelSetting.hpp>

#include "CubismModelSettingJson.hpp"
#include "Engine/TextureRenderTarget2D.h"
#include "L2DCubismRenderNormal.h"
#include "Motion/CubismMotion.hpp"
#include "Engine/TextureRenderTarget2D.h"
#include "L2DCubismAsset.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogL2DCubism, Log, All);

/**
 * 
 */
UCLASS()
class L2DCUBISM_API UL2DCubismAsset : public UObject
{
    GENERATED_UCLASS_BODY()

	CubismModelSettingJson* ModelSetting = nullptr;

    UPROPERTY()
    FString AssetName;

    UPROPERTY()
    TArray<uint8> ModelData;

//  UPROPERTY()
    TArray<TArray<uint8>> ModelExpression;

    UPROPERTY()
    TArray<FString> ModelExpressionName;

    UPROPERTY()
    TArray<uint8> ModelPhysics;

    UPROPERTY()
    TArray<uint8> ModelPose;

    UPROPERTY()
    TArray<uint8> ModelUserData;

//  UPROPERTY()
    TMap<FString, TArray<uint8>> MotionData;

    UPROPERTY()
    TArray<FString> TextureFileNames;

    UPROPERTY()
    TArray<uint8> ModelSettingJson;

    UPROPERTY()
    FString ContentLocation;
    
    void BeginDestroy() override;
    FString GetTexturePath();

public:
    
    UPROPERTY()
    FString ModelHomeDir;
    
    UPROPERTY()
    FString FileName;

//  UPROPERTY()
//  FString ContentRootDir;

    FString GetName();

    void SetModel(TArray<uint8> RawData);
    TArray<uint8> GetModel();

    void SetExpression(int i, TArray<uint8> RawData);
    TArray<uint8> GetExpression(int i);

    void SetPhysics(TArray<uint8> RawData);
    TArray<uint8> GetPhysics();

    void SetPose(TArray<uint8> RawData);
    TArray<uint8> GetPose();
    
    void SetUserData(TArray<uint8> RawData);
    TArray<uint8> GetUserData();

    void SetMotion(FString MotionFileName, TArray<uint8> RawData);
    TArray<uint8> GetMotion(FString MotionFileName);
    
    ICubismModelSetting* GetModelSetting();

//  CubismModel* GetCubismModel();

    virtual void Serialize(FArchive& Ar) override;

    void SetTextureFileName(csmInt32 i, const FString& TexttureFileName);
    void ParseModelSettingJson();

    void SetModelSettingJson(TArray<uint8> RawData);
    CubismModelSettingJson* GetModelSettingJson();
};
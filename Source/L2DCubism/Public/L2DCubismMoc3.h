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
#include "L2DCubismMoc3.generated.h"

// DECLARE_LOG_CATEGORY_EXTERN(LogL2DCubism, Log, All);

/**
 * 
 */
UCLASS()
class L2DCUBISM_API UL2DCubismMoc3 : public UObject
{
    GENERATED_UCLASS_BODY()

    UPROPERTY()
    TArray<uint8> ModelData;

    void BeginDestroy() override;

public:
    
    void SetMoc3(TArray<uint8> RawData);
    TArray<uint8> GetMoc3();
};
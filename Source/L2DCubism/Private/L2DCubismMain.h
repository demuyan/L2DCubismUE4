// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#pragma once

#include <CubismFramework.hpp>
#include <Model/CubismUserModel.hpp>
#include <ICubismModelSetting.hpp>

#include "CubismModelSettingJson.hpp"
#include "Engine/TextureRenderTarget2D.h"
#include "L2DCubismRenderNormal.h"
#include "L2DCubismAsset.h"
#include "L2DCubismMoc3.h"
#include "Motion/CubismMotion.hpp"

namespace L2DCubism
{
    enum class Priority
    {
        None = 0,
        Idle,
        Normal,
        Force
    };
}

class L2DCubismMain : public CubismUserModel
{
public:
    L2DCubismMain();
    ~L2DCubismMain();
//  void LoadAssets(FString dir, FString fileName);
    void SetAsset(UL2DCubismAsset* _Asset);
    void Update(float_t deltaTimeSec);
    void StartIdleMotion();
    CubismMotionQueueEntryHandle StartMotion(FString group, int32_t no, int32_t priority,
                                             ACubismMotion::FinishedMotionCallback onFinishedMotionHandler = nullptr);
    CubismMotionQueueEntryHandle StartRandomMotion(FString group, int32_t priority,
                                                   ACubismMotion::FinishedMotionCallback onFinishedMotionHandler =
                                                       nullptr);
    L2DCubismRenderNormal* GetRendererInstance();
    void SetExpression(FString expressionID);
    void SetRandomExpression();
    void MotionEventFired(const csmString& eventValue) override;
    virtual csmBool HitTest(FString hitAreaName, float_t x, float_t y);

    void SetTextureRenderTarget2D(UTextureRenderTarget2D* p);
    UTextureRenderTarget2D* GetTextureRenderTarget2D() const;
    void SetHighPrecisionMask(bool p);
    bool GetHighPrecisionMask();
    void Draw(CubismMatrix44& matrix);

    const FString MotionGroup_Idle = FString(TEXT("Idle"));

protected:
    void Init();
    void DoDraw();
    void PrepareModel(UL2DCubismAsset* _Asset);
    const CubismId* GetParamId(const csmChar* paramName);
    FString GetName(FString group, size_t i);
    void GetMotion(FString group, size_t i, FString MotioinFileName, CubismMotion*& Motion);
    void PreloadMotionGroup(FString group);
    void ReleaseMotions();
    void ReleaseExpressions();
    void ReleaseMotionGroup(const csmChar* group) const;
    void UpdateMotion(float_t deltaSec, bool& bMotionUpdated);
    void UpdateBreath(float_t deltaSec);
    void UpdatePhysics(float_t deltaSec);
    void UpdatePose(float_t deltaSec);
    void UpdateEyeBlink(float_t deltaSec);
    void UpdateExpression(float_t Float);
    void SetupTextures();
    UTexture2D* LoadTextureFromPath(const FString & Path);
    UL2DCubismMoc3* LoadMoc3FromPath(const FString& Path);

    UL2DCubismAsset* CubismAsset=nullptr;

private:
    UPROPERTY()
    ICubismModelSetting* ModelSetting;

    FString ModelHomeDir;
    double UserTimeSec;

    TMap<FString, ACubismMotion*> Expressions;
    TMap<FString, ACubismMotion*> Motions;
    csmVector<CubismIdHandle> EyeBlinkIds;
    csmVector<CubismIdHandle> LipSyncIds;

    UTextureRenderTarget2D* TextureRenderTarget2D = nullptr;
    bool bUseHighPrecisionMask = false;
};

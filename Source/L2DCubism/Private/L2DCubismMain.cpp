// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubismMain.h"
#include <fstream>
#include <vector>
#include <CubismModelSettingJson.hpp>
#include <Motion/CubismMotion.hpp>
#include <Physics/CubismPhysics.hpp>
#include <CubismDefaultParameterId.hpp>
#include <Utils/CubismString.hpp>
#include <Id/CubismIdManager.hpp>
#include <Motion/CubismMotionQueueEntry.hpp>
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "L2DCubismMoc3.h"
#include "Misc/FileHelper.h"
#include "L2DCubism/Public/L2DCubism.h"

// using namespace Framework;
using namespace L2DCubism;

L2DCubismMain::L2DCubismMain()
    : CubismUserModel()
{
    Init();
}

void L2DCubismMain::Init()
{
    ModelSetting = nullptr;
    UserTimeSec = 0.0f;
}

L2DCubismMain::~L2DCubismMain()
{
	ReleaseMotions();
	ReleaseExpressions();

	// for (csmInt32 i = 0; i < ModelSetting->GetMotionGroupCount(); i++)
	// {
	// 	const csmChar* group = ModelSetting->GetMotionGroupName(i);
	// 	ReleaseMotionGroup(group);
	// }
	
	delete ModelSetting;
}

void L2DCubismMain::SetAsset(UL2DCubismAsset* _Asset)
{
    PrepareModel(_Asset);

    CreateRenderer();
    SetupTextures();
}

void L2DCubismMain::UpdateEyeBlink(float_t deltaSec)
{
    if (_eyeBlink != nullptr)
    {
        _eyeBlink->UpdateParameters(_model, deltaSec);
    }
}

void L2DCubismMain::UpdateExpression(float_t deltaSec)
{
    if (_expressionManager != nullptr)
    {
        _expressionManager->UpdateMotion(_model, deltaSec);
    }
}

void L2DCubismMain::PrepareModel(UL2DCubismAsset* _Asset)
{
    _updating = true;
    _initialized = false;

    if (_Asset == nullptr)
    {
        return;
    }

    CubismAsset = _Asset;
    ModelSetting = _Asset->GetModelSetting();

    TArray<uint8> Data;

    FString ModelFileName(ModelSetting->GetModelFileName());
    if (0 < ModelFileName.Len())
    {
        FString Path = FPaths::GetPath(CubismAsset->GetPathName());
        // Load moc3
        UL2DCubismMoc3* Moc3 = LoadMoc3FromPath(FPaths::Combine(Path, FPaths::GetBaseFilename(ModelFileName)));
        Data = Moc3->GetMoc3();
        LoadModel(Data.GetData(), Data.Num());
    }

    if (0 < ModelSetting->GetExpressionCount())
    {
        for (csmInt32 i = 0; i < ModelSetting->GetExpressionCount(); i++)
        {
            Data = _Asset->GetExpression(i);
            FString ExpressionName(ModelSetting->GetExpressionName(i));
            ACubismMotion* Motion = LoadExpression(Data.GetData(), Data.Num(), TCHAR_TO_ANSI(*ExpressionName));

            if (Expressions.Contains(ExpressionName))
            {
                ACubismMotion::Delete(Expressions[ExpressionName]);
                Expressions.Remove(ExpressionName);
            }
            Expressions[ExpressionName] = Motion;
        }
    }
    
    FString PhysicsFileName(ModelSetting->GetPhysicsFileName());
    if (0 < PhysicsFileName.Len())
    {
	    UE_LOG(LogL2DCubism, Log, TEXT("[APP]Physics"));

        Data = _Asset->GetPhysics();
        LoadPhysics(Data.GetData(), Data.Num());
    }
    
    const FString PoseFileName(ModelSetting->GetPoseFileName());
    if (0 < PoseFileName.Len())
    {
	    UE_LOG(LogL2DCubism, Log, TEXT("[APP]Pose"));

        Data = _Asset->GetPose();
        LoadPose(Data.GetData(), Data.Num());
    }

    if (0 < ModelSetting->GetEyeBlinkParameterCount())
    {
		UE_LOG(LogL2DCubism, Log, TEXT("[APP]EyeBlinkParameter"));

        _eyeBlink = CubismEyeBlink::Create(ModelSetting);
    }

    {
        _breath = CubismBreath::Create();
        csmVector<CubismBreath::BreathParameterData> BreathParams;
        BreathParams.PushBack(
            CubismBreath::BreathParameterData(GetParamId(DefaultParameterId::ParamAngleX), 0.0f, 15.0f, 6.5345f, 0.5f));
        BreathParams.PushBack(
            CubismBreath::BreathParameterData(GetParamId(DefaultParameterId::ParamAngleY), 0.0f, 8.0f, 3.5345f, 0.5f));
        BreathParams.PushBack(
            CubismBreath::BreathParameterData(GetParamId(DefaultParameterId::ParamAngleZ), 0.0f, 10.0f, 5.5345f, 0.5f));
        BreathParams.PushBack(
            CubismBreath::BreathParameterData(GetParamId(DefaultParameterId::ParamBodyAngleX), 0.0f, 4.0f, 15.5345f,
                                              0.5f));
        BreathParams.PushBack(
            CubismBreath::BreathParameterData(GetParamId(DefaultParameterId::ParamBreath), 0.5f, 0.5f, 3.2345f, 0.5f));

        _breath->SetParameters(BreathParams);
    }

    FString UserDataFile(ModelSetting->GetUserDataFile());
    if (0 < PoseFileName.Len())
    {
        UE_LOG(LogL2DCubism, Log, TEXT("[APP]UserData"));

        Data = _Asset->GetUserData();
        LoadUserData(Data.GetData(), Data.Num());
    }

    for (csmInt32 i = 0; i < ModelSetting->GetEyeBlinkParameterCount(); ++i)
    {
    	EyeBlinkIds.PushBack(ModelSetting->GetEyeBlinkParameterId(i));
    }

    // LipSyncIds
    for (csmInt32 i = 0; i < ModelSetting->GetLipSyncParameterCount(); ++i)
    {
        LipSyncIds.PushBack(ModelSetting->GetLipSyncParameterId(i));
    }

    //Layout
    csmMap<csmString, csmFloat32> layout;
    ModelSetting->GetLayoutMap(layout);
    _modelMatrix->SetupFromLayout(layout);

    _model->SaveParameters();

    for (csmInt32 i = 0; i < ModelSetting->GetMotionGroupCount(); i++)
    {
        FString Group(ModelSetting->GetMotionGroupName(i));
        PreloadMotionGroup(Group);
    }

    _motionManager->StopAllMotions();

    _updating = false;
    _initialized = true;
}

inline const CubismId* L2DCubismMain::GetParamId(const csmChar* paramName)
{
    return CubismFramework::GetIdManager()->GetId(paramName);
}

FString L2DCubismMain::GetName(FString group, size_t i)
{
    return FString::Format(TEXT("{0}_{1}"), {group, i});
}

void L2DCubismMain::GetMotion(FString group, size_t i, FString FullPath, CubismMotion*& Motion)
{
    TArray<uint8> Data;

    FString Name = GetName(group, i);
    Data = CubismAsset->GetMotion(Name);
    
    Motion = static_cast<CubismMotion*>(CubismUserModel::LoadMotion(Data.GetData(), Data.Num(), TCHAR_TO_ANSI(*Name)));
    float_t FadeInSec = ModelSetting->GetMotionFadeInTimeValue(TCHAR_TO_ANSI(*group), i);
    if (0.0f <= FadeInSec)
    {
        Motion->SetFadeInTime(FadeInSec);
    }

    float_t FadeOutSec = ModelSetting->GetMotionFadeInTimeValue(TCHAR_TO_ANSI(*group), i);
    if (0.0f <= FadeOutSec)
    {
        Motion->SetFadeOutTime(FadeOutSec);
    }

    Motion->SetEffectIds(EyeBlinkIds, LipSyncIds);
}

void L2DCubismMain::PreloadMotionGroup(FString group)
{
    for (csmInt32 i = 0; i < ModelSetting->GetMotionCount(TCHAR_TO_ANSI(*group)); ++i)
    {
        const FString MotionFileName(ModelSetting->GetMotionFileName(TCHAR_TO_ANSI(*group), i));
        FString Name = GetName(group, i);

        CubismMotion* Motion;
        GetMotion(group, i, MotionFileName, Motion);

        if (Motions.Contains(Name))
        {
            ACubismMotion::Delete(Motions[Name]);
        }
        Motions.Add(Name, Motion);
    }
}

void L2DCubismMain::ReleaseMotions()
{
    for (auto It = Motions.CreateConstIterator(); It; ++It)
    {
        ACubismMotion::Delete(It->Value);
    }
    Motions.Empty();
}

void L2DCubismMain::ReleaseExpressions()
{
    for (auto It = Expressions.CreateConstIterator(); It; ++It)
    {
        ACubismMotion::Delete(It->Value);
    }
    Expressions.Empty();
}

// void L2DCubismMain::ReleaseMotionGroup(const csmChar* group) const
// {
// 	const csmInt32 count = ModelSetting->GetMotionCount(group);
// 	for (csmInt32 i = 0; i < count; i++)
// 	{
// 		csmString voice = ModelSetting->GetMotionSoundFileName(group, i);
// 		if (strcmp(voice.GetRawString(), "") != 0)
// 		{
// 			csmString path = voice;
// 			path = ModelSetting + path;
// 		}
// 	}
// }

void L2DCubismMain::UpdateMotion(float_t deltaSec, bool& bMotionUpdated)
{
    _model->LoadParameters();
    if (_motionManager->IsFinished())
    {
        StartIdleMotion();
    }
    else
    {
        bMotionUpdated = _motionManager->UpdateMotion(_model, deltaSec);
    }
    _model->SaveParameters();
}

void L2DCubismMain::UpdateBreath(float_t deltaSec)
{
    if (_breath != nullptr)
    {
        _breath->UpdateParameters(_model, deltaSec);
    }
}

void L2DCubismMain::UpdatePhysics(float_t deltaSec)
{
    if (_physics != nullptr)
    {
        _physics->Evaluate(_model, deltaSec);
    }
}

void L2DCubismMain::UpdatePose(float_t deltaSec)
{
    if (_pose != nullptr)
    {
        _pose->UpdateParameters(_model, deltaSec);
    }
}

void L2DCubismMain::Update(float_t deltaSec)
{
    UserTimeSec += deltaSec;
    bool bMotionUpdated = false;

    UpdateMotion(deltaSec, bMotionUpdated);
    if (bMotionUpdated)
    {
        UpdateEyeBlink(deltaSec);
    }
    UpdateExpression(deltaSec);
    UpdateBreath(deltaSec);
    UpdatePhysics(deltaSec);
    UpdatePose(deltaSec);

    _model->Update();
}

void L2DCubismMain::StartIdleMotion()
{
    StartRandomMotion(MotionGroup_Idle, static_cast<int>(Priority::Idle));
}

CubismMotionQueueEntryHandle L2DCubismMain::StartMotion(FString group, int32_t no, int32_t priority,
                                                        ACubismMotion::FinishedMotionCallback onFinishedMotionHandler)
{
    if (priority == static_cast<int>(Priority::Force))
    {
        _motionManager->SetReservePriority(priority);
    }
    else if (!_motionManager->ReserveMotion(priority))
    {
        return InvalidMotionQueueEntryHandleValue;
    }

    const FString MotionFileName(ModelSetting->GetMotionFileName(TCHAR_TO_ANSI(*group), no));
    FString Name = FString::Format(TEXT("{0}_{1}"), {group, no});

    CubismMotion* Motion = static_cast<CubismMotion*>(Motions[TCHAR_TO_ANSI(*Name)]);
    bool bAutoDelete = false;
    if (Motion == nullptr)
    {
        FString FullPath = ModelHomeDir + MotionFileName;
        GetMotion(group, no, FullPath, Motion);
        bAutoDelete = true;
    }
    else
    {
        Motion->SetFinishedMotionHandler(onFinishedMotionHandler);
    }
    return _motionManager->StartMotionPriority(Motion, bAutoDelete, priority);
}


CubismMotionQueueEntryHandle L2DCubismMain::StartRandomMotion(const FString group, int32_t priority,
                                                              ACubismMotion::FinishedMotionCallback
                                                              onFinishedMotionHandler)
{
    if (ModelSetting->GetMotionCount(TCHAR_TO_ANSI(*group)) == 0)
    {
        return InvalidMotionQueueEntryHandleValue;
    }

    int32_t No = FMath::RandRange(0, ModelSetting->GetMotionCount(TCHAR_TO_ANSI(*group)) - 1);
    return StartMotion(group, No, priority, onFinishedMotionHandler);
}

L2DCubismRenderNormal* L2DCubismMain::GetRendererInstance()
{
	// ここでエラーが発生したい場合は、CubismUserModel.hppの292行目をprivateからpublicに変更する
    return static_cast<L2DCubismRenderNormal*>(_renderer); 
}

void L2DCubismMain::DoDraw()
{
    GetRendererInstance()->DrawModel();
    GetRendererInstance()->UseHighPrecisionMask(bUseHighPrecisionMask);
}

void L2DCubismMain::Draw(CubismMatrix44& matrix)
{
    L2DCubismRenderNormal* renderer = GetRendererInstance();

	if (_model == nullptr || renderer == nullptr || TextureRenderTarget2D == nullptr)
	{
        return;
    }

    renderer->SetTextureRenderTarget2D(TextureRenderTarget2D);

    matrix.MultiplyByMatrix(_modelMatrix);
    renderer->SetMvpMatrix(&matrix);

    DoDraw();
}

csmBool L2DCubismMain::HitTest(const FString hitAreaName, float_t x, float_t y)
{
    if (_opacity < 1)
    {
        return false;
    }
    const int32_t count = ModelSetting->GetHitAreasCount();
    for (int32_t i = 0; i < count; i++)
    {
        FString CompStr(ModelSetting->GetHitAreaName(i));
        if (hitAreaName.Compare(CompStr) == 0)
        {
            const CubismIdHandle drawID = ModelSetting->GetHitAreaId(i);
            return IsHit(drawID, x, y);
        }
    }
    return false;
}

void L2DCubismMain::SetExpression(FString expressionID)
{
    ACubismMotion* motion = Expressions[TCHAR_TO_ANSI(*expressionID)];

    if (motion != nullptr)
    {
        _expressionManager->StartMotionPriority(motion, false, static_cast<int>(Priority::Force));
    }
}

void L2DCubismMain::SetRandomExpression()
{
    if (Expressions.Num() == 0)
    {
        return;
    }

    int No = FMath::RandRange(0, Expressions.Num() - 1);
    TArray<FString> Keys;
    Expressions.GetKeys(Keys);
    SetExpression(TCHAR_TO_ANSI(*Keys[No]));
}

UTexture2D* L2DCubismMain::LoadTextureFromPath(const FString& Path)
{
    if (Path.IsEmpty()) return nullptr;

    return Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(Path)));
}

UL2DCubismMoc3* L2DCubismMain::LoadMoc3FromPath(const FString& Path)
{
    if (Path.IsEmpty()) return nullptr;

    return Cast<UL2DCubismMoc3>(StaticLoadObject(UL2DCubismMoc3::StaticClass(), NULL, *(Path)));
}

void L2DCubismMain::SetupTextures()
{
    FString PathName = FPaths::GetPath(CubismAsset->GetPathName());
    
    for (csmInt32 ModelTextureNumber = 0; ModelTextureNumber < ModelSetting->GetTextureCount(); ModelTextureNumber++)
    {
        FString StrFileName(ModelSetting->GetTextureFileName(ModelTextureNumber));
        FString TextureFileName = FPaths::GetBaseFilename(StrFileName, false).Replace(TEXT("."), TEXT("_"));
        
        if (TextureFileName.Len() <= 0)
        {
            continue;
        }
        
        UTexture2D* LoadedImage = LoadTextureFromPath(FPaths::Combine(PathName, TextureFileName));
        if (IsValid(LoadedImage))
        {
            GetRendererInstance()
                ->BindTexture(ModelTextureNumber, LoadedImage);
        }
        LoadedImage->AddToRoot();
    }
}

void L2DCubismMain::MotionEventFired(const csmString& eventValue)
{
    CubismLogInfo("%s is fired on L2DCubismMain!!", eventValue.GetRawString());
}

void L2DCubismMain::SetTextureRenderTarget2D(UTextureRenderTarget2D* p)
{
    TextureRenderTarget2D = p;
}

UTextureRenderTarget2D* L2DCubismMain::GetTextureRenderTarget2D() const
{
    return TextureRenderTarget2D;
}

void L2DCubismMain::SetHighPrecisionMask(bool p)
{
    bUseHighPrecisionMask = p;
}

bool L2DCubismMain::GetHighPrecisionMask()
{
    return bUseHighPrecisionMask;
}

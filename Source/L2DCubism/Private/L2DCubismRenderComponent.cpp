// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubismRenderComponent.h"
#include "L2DCubismAsset.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

UL2DCubismRenderComponent::UL2DCubismRenderComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    InitializeCubism();

    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneAsset(TEXT("/Engine/BasicShapes/Plane"));
    if (PlaneAsset.Succeeded())
    {
        FRotator Rotate;
        Rotate.Pitch = 0;
        Rotate.Yaw = 0;
        Rotate.Roll = 0;

        this->SetStaticMesh(PlaneAsset.Object);
        this->SetRelativeRotation(Rotate);
    }

    EnableComponentTick();
    bTickInEditor = true;
    bChangeAsset = false;
}

void UL2DCubismRenderComponent::BeginDestroy()
{
    Super::BeginDestroy();

    DisableComponentTick();
	_CubismModel = nullptr;
	CubismAsset = nullptr;
	delete _CubismMain;
}

void UL2DCubismRenderComponent::DisableComponentTick()
{
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bTickEvenWhenPaused = false;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    // Empty, but required because we don't want to have to include LightMap.h and ShadowMap.h in StaticMeshComponent.h, and they are required to compile FLightMapRef and FShadowMapRef
}

static void CubismLog(const char* message)
{
    UE_LOG(LogL2DCubism, Log, TEXT("%s"), ANSI_TO_TCHAR(message));
}

void UL2DCubismRenderComponent::InitializeCubism()
{
    //setup cubism
    CubismOption.LogFunction = CubismLog;
    CubismOption.LoggingLevel = CubismFramework::Option::LogLevel_Debug;

    CubismFramework::StartUp(&CubismAllocator, &CubismOption);

    //Initialize cubism
    CubismFramework::Initialize();
}

void UL2DCubismRenderComponent::PostInitProperties()
{
    Super::PostInitProperties();

    // if (CubismAsset) {
    //  InitCubismParameters();
    // }
}

void UL2DCubismRenderComponent::LoadCubismAsset()
{
    if (!IsValid(CubismAsset))
    {
        return;
    }

    InitParameters();
}

// パラメータ情報初期化
void UL2DCubismRenderComponent::InitParameters()
{
    CubismModel* Mdl = GetCubismModel();
    csmInt32 Count = Mdl->GetParameterCount();

    Mdl->LoadParameters();

    const char** parameterIds = csmGetParameterIds(Mdl->GetModel());
    for (csmInt32 i = 0; i < Count; ++i)
    {
        FCubismParameter Param;

        Param.Id = FString(UTF8_TO_TCHAR(parameterIds[i]));
        Param.MaximumValue = Mdl->GetParameterMaximumValue(i);
        Param.MinimumValue = Mdl->GetParameterMinimumValue(i);
        Param.DefaultValue = Mdl->GetParameterDefaultValue(i);
        Param.CurrentValue = Mdl->GetParameterValue(i);

        Parameters.Add(Param);
    }
}

L2DCubismMain* UL2DCubismRenderComponent::GetCubismMain()
{
    if (CubismAsset == nullptr)
        return nullptr;
    
    if (_CubismMain == nullptr)
    {
        _CubismMain = new L2DCubismMain();
        _CubismMain->SetAsset(CubismAsset);

        UE_LOG(LogL2DCubism, Log, TEXT("New L2DCubismMain"));
    }
    return _CubismMain;
}


CubismModel* UL2DCubismRenderComponent::GetCubismModel()
{
    if (_CubismModel == nullptr)
    {
        _CubismModel = GetCubismMain()->GetModel();
    }
    return _CubismModel;
}

// 毎フレーム読み取り
void UL2DCubismRenderComponent::ReadParameters()
{
    CubismModel* Mdl = GetCubismModel();
    csmInt32 Count = Mdl->GetParameterCount();

    for (csmInt32 i = 0; i < Count; ++i)
    {
        Parameters[i].CurrentValue = Mdl->GetParameterValue(i);
    }
}

void UL2DCubismRenderComponent::WriteParameters()
{
    CubismModel* Mdl = GetCubismModel();
    csmInt32 Count = Mdl->GetParameterCount();

    for (csmInt32 i = 0; i < Count; ++i)
    {
        Mdl->SetParameterValue(i, Parameters[i].CurrentValue, 1);
    }
    Mdl->SaveParameters();
}

void UL2DCubismRenderComponent::PostLoad()
{
    Super::PostLoad();

    // if (CubismAsset) {
    //  InitCubismParameters();
    // }
}

#if WITH_EDITOR
void UL2DCubismRenderComponent::PostEditChangeChainProperty(
    struct FPropertyChangedChainEvent& PropertyChangedChainEvent)
{
    // if (PropertyChangedChainEvent.GetPropertyName() == "CurrentValue") {
    //
    //  int32 index = PropertyChangedChainEvent.GetArrayIndex(TEXT("CubismParameters"));
    //
    //  FCubismParameter Param = CubismParameters[index];
    //
    //  CubismAsset->SetParameter(Param.Id, Param.CurrentValue);
    //  CubismAsset->UpdateDrawables();
    //
    //  //  Param.CurrentValue = FMath::Clamp(Param.CurrentValue, Param.MinimumValue, Param.MaximumValue);
    //
    // }

    if (PropertyChangedChainEvent.GetPropertyName() == "CubismAsset")
    {
        LoadCubismAsset();
        bChangeAsset = true;
    }

	if (PropertyChangedChainEvent.GetPropertyName() == "RenderTarget2D")
	{
		bChangeRenderTarget2D = true;
	}

    Super::PostEditChangeChainProperty(PropertyChangedChainEvent);
}
#endif

bool UL2DCubismRenderComponent::IsEditing()
{
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        UWorld* World = Context.World();
        if (World == nullptr || (Context.WorldType != EWorldType::PIE && Context.WorldType != EWorldType::Game))
        {
            continue;
        }

        switch (Context.WorldType)
        {
        case EWorldType::Game:
            UE_LOG(LogL2DCubism, Log, TEXT("World is Game"));
            return false;

        case EWorldType::PIE:
            UE_LOG(LogL2DCubism, Log, TEXT("World is PIE"));
            return false;

        default:
            break;
        }
    }
    return true;
}

void UL2DCubismRenderComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	if (!IsValid(CubismAsset) || !IsValid(RenderTarget2D))
	{
        return;
    }

    PassingSeconds += DeltaTime;
    if (PassingSeconds < NextUpdateSeconds)
    {
        return;
    }

    const float UpdateInvervalSeconds = ((TickRate > 0) ? (1.0f / TickRate) : 0.05f);
    
    if (Model == nullptr || bChangeAsset == true)
    {
        Model = GetCubismMain();

        Model->SetTextureRenderTarget2D(RenderTarget2D);
        Model->SetHighPrecisionMask(HighPrecisionMask);

        bChangeAsset = false;
    }

    CubismMatrix44 projection = CubismMatrix44();

    if (IsEditing())
    {
        PassingSeconds = LastUpdateSeconds;
        WriteParameters();
    }

    Model->Update(PassingSeconds - LastUpdateSeconds);
    LastUpdateSeconds = PassingSeconds;

    Model->Draw(projection);

    if (IsEditing() == false)
    {
        ReadParameters();
    }

    NextUpdateSeconds = PassingSeconds + UpdateInvervalSeconds; 
}

void UL2DCubismRenderComponent::EnableComponentTick()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bTickEvenWhenPaused = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

namespace
{
    void EventFinishedMotion(ACubismMotion* self)
    {
        UE_LOG(LogL2DCubism, Log, TEXT("call EventFinishedMotion"));
    }
}

void UL2DCubismRenderComponent::StartMotion(const FString& Group, int32 No, int32 Priority)
{
    if (Model == nullptr)
    {
        return;
    }

    Model->StartMotion(Group, No, Priority, EventFinishedMotion);
}

void UL2DCubismRenderComponent::StartRandomMotion(const FString& Group, int32 Priority)
{
    if (Model == nullptr)
    {
        return;
    }

    Model->StartRandomMotion(Group, Priority, EventFinishedMotion);
}

void UL2DCubismRenderComponent::SetExpression(const FString& ExpresionId)
{
    if (Model == nullptr)
    {
        return;
    }
    Model->SetExpression(ExpresionId);
}

void UL2DCubismRenderComponent::SetRandomExpression()
{
    if (Model == nullptr)
    {
        return;
    }
    Model->SetRandomExpression();
}


// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#pragma once

#include "CoreMinimal.h"
#include "L2DCubismMain.h"
#include "Components/StaticMeshComponent.h"
#include "L2DCubismAsset.h"
#include "Tickable.h"
#include "L2DCubismMemoryAllocator.hpp"

#include "L2DCubismRenderComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFinishedMotion);

USTRUCT(BlueprintType)
struct FCubismParameter
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(Category = CubismPararameters, VisibleAnywhere, BlueprintReadOnly)
    FString Id;

    UPROPERTY(Category = CubismPararameters, BlueprintReadOnly)
    float MaximumValue;
    
    UPROPERTY(Category = CubismPararameters, BlueprintReadOnly)
    float MinimumValue;

    UPROPERTY(Category = CubismPararameters,  BlueprintReadOnly)
    float DefaultValue;

    UPROPERTY(Category = CubismPararameters, EditAnywhere, BlueprintReadWrite)
    float CurrentValue;
};

/**
 * 
 */
UCLASS(ClassGroup = Rendering, meta = (BlueprintSpawnableComponent))
class L2DCUBISM_API UL2DCubismRenderComponent : public UStaticMeshComponent
{
    GENERATED_UCLASS_BODY()

public:

    UPROPERTY(Category = Cubism, EditAnywhere, BlueprintReadOnly)
    class UTextureRenderTarget2D* RenderTarget2D = nullptr;

    UPROPERTY(Category = Cubism, EditAnywhere, BlueprintReadOnly)
    UL2DCubismAsset* CubismAsset= nullptr;
    
    UPROPERTY(Category = Cubism, EditAnywhere, BlueprintReadOnly)
    TArray<FCubismParameter> Parameters;

    UPROPERTY(Category = Cubism, EditAnywhere, BlueprintReadOnly)
    bool HighPrecisionMask;

    UPROPERTY(Category = Cubism, EditAnywhere)
    int TickRate = 30;

    UFUNCTION(Category = CubismModel, BlueprintCallable)
    void StartMotion(const FString& Group, int32 No, int32 Priority);

    UFUNCTION(Category = CubismModel, BlueprintCallable)
    void StartRandomMotion(const FString& Group, int32 Priority);

    UFUNCTION(Category = CubismModel, BlueprintCallable)
    void SetExpression(const FString& ExpresionId);

    UFUNCTION(Category = CubismModel, BlueprintCallable)
    void SetRandomExpression();

    // TODO:‚ ‚Æ‚Ü‚í‚µ
    // UPROPERTY(Category = CubismModel, BlueprintAssignable)
    // FFinishedMotion OnFinishedMotion;
    //
    // void FinishedMotion(ACubismMotion* self)
    // {
    //  OnFinishedMotion.Broadcast();
    // }

private:
    
    // TODO: ‚ ‚Æ‚Ü‚í‚µ
    // FFinishedMotion FinishedMotionDelegate;

    L2DCubismMain* Model = nullptr;
    CubismModel* _CubismModel = nullptr;
    L2DCubismMain* _CubismMain = nullptr;
    
    L2DCubismMemoryAllocator CubismAllocator;      ///< Cubism SDK Allocator
    Csm::CubismFramework::Option CubismOption;  ///< Cubism SDK Option

    bool bChangeAsset = false;
	bool bChangeRenderTarget2D = false;

    float PassingSeconds = 0.0f;
    float NextUpdateSeconds = 0.0f;
    float LastUpdateSeconds = 0.0f;
    
    void InitializeCubism();
    
    virtual void PostInitProperties() override;

    void LoadCubismAsset();
    void InitParameters();
    void ReadParameters();
    void WriteParameters();
#if WITH_EDITOR
    void PostEditChangeChainProperty(
        struct  FPropertyChangedChainEvent & PropertyChangedEvent
    ) override;
#endif
    bool IsEditing();

    virtual void PostLoad() override;

    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
        
    void EnableComponentTick();
    void BeginDestroy() override;
    void DisableComponentTick();

    CubismModel* GetCubismModel();
    L2DCubismMain* GetCubismMain();
};
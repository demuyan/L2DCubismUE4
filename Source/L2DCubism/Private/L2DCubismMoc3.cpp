// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubismMoc3.h"

UL2DCubismMoc3::UL2DCubismMoc3(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ModelData = TArray<uint8>();
}

void UL2DCubismMoc3::BeginDestroy()
{
    Super::BeginDestroy();

//  CubismFramework::Dispose();
}

TArray<uint8> UL2DCubismMoc3::GetMoc3()
{
    return ModelData;
}

void UL2DCubismMoc3::SetMoc3(TArray<uint8> RawData)
{
    ModelData = RawData;
}


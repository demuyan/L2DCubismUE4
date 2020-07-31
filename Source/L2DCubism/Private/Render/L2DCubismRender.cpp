// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubismRender.h"

L2DCubismRender::L2DCubismRender()
{
}

L2DCubismRender::~L2DCubismRender()
{
}

void L2DCubismRender::Initialize(CubismModel* model)
{
}

void L2DCubismRender::SetCulling(bool culling)
{
    bCulling = culling;
}

bool L2DCubismRender::IsCulling() const
{
    return bCulling;
}

FMatrix L2DCubismRender::ConvertToFMatrix(CubismMatrix44& InCubismMartix)
{
    FMatrix OutMatrix;
    auto CubismMatirix = InCubismMartix.GetArray();

    for (size_t j = 0; j < 4; j++)
    {
        for (size_t i = 0; i < 4; i++)
        {
            OutMatrix.M[j][i] = *CubismMatirix;
            ++CubismMatirix;
        }
    }
    return OutMatrix;
}

// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubismShader.h"

// mask generation
IMPLEMENT_SHADER_TYPE(, L2DCubismVertSetupMask, TEXT("/Shader/L2DCubismShader.usf"), TEXT("SetupMaskVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, L2DCubismPixelSetupMask, TEXT("/Shader/L2DCubismShader.usf"), TEXT("SetupMaskPS"), SF_Pixel)
// VertexShader
IMPLEMENT_SHADER_TYPE(, L2DCubismVertNormal, TEXT("/Shader/L2DCubismShader.usf"), TEXT("NormalVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, L2DCubismVertMasked, TEXT("/Shader/L2DCubismShader.usf"), TEXT("MaskedVS"), SF_Vertex)
// PixelShader
IMPLEMENT_SHADER_TYPE(, L2DCubismPixelNormal, TEXT("/Shader/L2DCubismShader.usf"), TEXT("NormalPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, L2DCubismPixelMasked, TEXT("/Shader/L2DCubismShader.usf"), TEXT("MaskedPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, L2DCubismPixelMaskedInverted, TEXT("/Shader/L2DCubismShader.usf"), TEXT("MaskedInvertedPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, L2DCubismPixelNormalPremult, TEXT("/Shader/L2DCubismShader.usf"), TEXT("NormalPremultPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, L2DCubismPixelMaskedPremult, TEXT("/Shader/L2DCubismShader.usf"), TEXT("MaskedPremultPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, L2DCubismPixelMaskedInvertedPremult, TEXT("/Shader/L2DCubismShader.usf"), TEXT("MaskedInvertedPremultPS"), SF_Pixel)


// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#pragma once

#include "CoreMinimal.h"
#include "ShaderParameters.h"
#include "Shader.h"
#include "GlobalShader.h"
#include "ShaderParameterUtils.h"

class FL2DCubismShader : public FGlobalShader
{
public:
    FL2DCubismShader() {}

    FL2DCubismShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        ProjectMatrix.Bind(Initializer.ParameterMap, TEXT("projectMatrix"));
        ClipMatrix.Bind(Initializer.ParameterMap, TEXT("clipMatrix"));
        BaseColor.Bind(Initializer.ParameterMap, TEXT("baseColor"));
        ChannelFlag.Bind(Initializer.ParameterMap, TEXT("channelFlag") );

        MainTexture.Bind(Initializer.ParameterMap, TEXT("mainTexture"));
        MainSampler.Bind(Initializer.ParameterMap, TEXT("mainSampler"));
        MaskTexture.Bind(Initializer.ParameterMap, TEXT("maskTexture"));
    }

    template<typename TShaderRHIParamRef>
    void SetParameters(
        FRHICommandList& RHICmdList, 
        const TShaderRHIParamRef ShaderRHI, 
        const FMatrix& InProjectMatrix,
        const FMatrix& InClipMatrix,
        const FVector4& InBaseColor,
        const FVector4& InChannelFlag,
        FTextureRHIRef InMainTexture,
        FTextureRHIRef InMaskTexture
        )
    {
        SetShaderValue(RHICmdList, ShaderRHI, ProjectMatrix, InProjectMatrix);
        SetShaderValue(RHICmdList, ShaderRHI, ClipMatrix, InClipMatrix);
        SetShaderValue(RHICmdList, ShaderRHI, BaseColor, InBaseColor);
        SetShaderValue(RHICmdList, ShaderRHI, ChannelFlag, InChannelFlag);

        SetTextureParameter(RHICmdList, ShaderRHI, MainTexture, InMainTexture);
        SetSamplerParameter(RHICmdList, ShaderRHI, MainSampler, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());
        SetTextureParameter(RHICmdList, ShaderRHI, MaskTexture, InMaskTexture);
    }
    
    // FShader interface.
    virtual bool Serialize(FArchive& Ar) override
    {
        bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
        Ar  << ProjectMatrix
            << ClipMatrix
            << BaseColor
            << ChannelFlag
            << MainTexture
            << MainSampler;
        Ar << MaskTexture;

        return bShaderHasOutdatedParameters;
    }
    
    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
    }
    
protected:
    FShaderParameter ProjectMatrix;
    FShaderParameter ClipMatrix;
    FShaderParameter BaseColor;
    FShaderParameter ChannelFlag;

    FShaderResourceParameter MainTexture;
    FShaderResourceParameter MainSampler;
    FShaderResourceParameter MaskTexture;
};

class FL2DCubismMaskShader : public FGlobalShader
{
public:
    FL2DCubismMaskShader() {}

    FL2DCubismMaskShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        ProjectMatrix.Bind(Initializer.ParameterMap, TEXT("projectMatrix"));
        BaseColor.Bind(Initializer.ParameterMap, TEXT("baseColor"));
        ChannelFlag.Bind(Initializer.ParameterMap, TEXT("channelFlag"));

        MainTexture.Bind(Initializer.ParameterMap, TEXT("mainTexture"));
        MainSampler.Bind(Initializer.ParameterMap, TEXT("mainSampler"));
    }

    template<typename TShaderRHIParamRef>
    void SetParameters(
        FRHICommandList& RHICmdList,
        const TShaderRHIParamRef ShaderRHI,
        const FMatrix& InProjectMatrix,
        const FVector4& InBaseColor,
        const FVector4& InChannelFlag,
        FTextureRHIRef InMainTexture
    )
    {
        SetShaderValue(RHICmdList, ShaderRHI, ProjectMatrix, InProjectMatrix);
        SetShaderValue(RHICmdList, ShaderRHI, BaseColor, InBaseColor);
        SetShaderValue(RHICmdList, ShaderRHI, ChannelFlag, InChannelFlag);

        SetTextureParameter(RHICmdList, ShaderRHI, MainTexture, InMainTexture);
        SetSamplerParameter(RHICmdList, ShaderRHI, MainSampler, TStaticSamplerState<SF_Bilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI());
    }

    // FShader interface.
    virtual bool Serialize(FArchive& Ar) override
    {
        bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
        Ar << ProjectMatrix;
        Ar << BaseColor;
        Ar << ChannelFlag;
        Ar << MainTexture;
        Ar << MainSampler;

        return bShaderHasOutdatedParameters;
    }

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }

    static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
    {
        FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
    }

protected:
    FShaderParameter ProjectMatrix;
    FShaderParameter BaseColor;
    FShaderParameter ChannelFlag;

    FShaderResourceParameter MainTexture;
    FShaderResourceParameter MainSampler;
};


class L2DCubismVertSetupMask : public FL2DCubismMaskShader
{
    DECLARE_SHADER_TYPE(L2DCubismVertSetupMask, Global);
public:
    L2DCubismVertSetupMask() {}

    L2DCubismVertSetupMask(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FL2DCubismMaskShader(Initializer)
    {}
};

class L2DCubismPixelSetupMask : public FL2DCubismMaskShader
{
    DECLARE_SHADER_TYPE(L2DCubismPixelSetupMask, Global);
public:
    L2DCubismPixelSetupMask() {}

    L2DCubismPixelSetupMask(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FL2DCubismMaskShader(Initializer)
    {}
};

class L2DCubismVertNormal : public FL2DCubismShader
{
    DECLARE_SHADER_TYPE(L2DCubismVertNormal, Global);
public:
    L2DCubismVertNormal() {}

    L2DCubismVertNormal(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FL2DCubismShader(Initializer)
    {}
};


class L2DCubismVertMasked : public FL2DCubismShader
{
    DECLARE_SHADER_TYPE(L2DCubismVertMasked, Global);
public:
    L2DCubismVertMasked() {}

    L2DCubismVertMasked(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FL2DCubismShader(Initializer)
    {}
};

class L2DCubismPixelNormal : public FL2DCubismShader
{
    DECLARE_SHADER_TYPE(L2DCubismPixelNormal, Global);
public:
    L2DCubismPixelNormal() {}

    L2DCubismPixelNormal(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FL2DCubismShader(Initializer)
    {
    }
};

class L2DCubismPixelMasked : public FL2DCubismShader
{
    DECLARE_SHADER_TYPE(L2DCubismPixelMasked, Global);
public:
    L2DCubismPixelMasked() {}

    L2DCubismPixelMasked(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FL2DCubismShader(Initializer)
    {
    }
};


class L2DCubismPixelMaskedInverted : public FL2DCubismShader
{
    DECLARE_SHADER_TYPE(L2DCubismPixelMaskedInverted, Global);
public:
    L2DCubismPixelMaskedInverted() {}

    L2DCubismPixelMaskedInverted(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FL2DCubismShader(Initializer)
    {
    }
};

 class L2DCubismPixelMaskedPremult : public FL2DCubismShader
 {
    DECLARE_SHADER_TYPE(L2DCubismPixelMaskedPremult, Global);
public:
    L2DCubismPixelMaskedPremult() {}

    L2DCubismPixelMaskedPremult(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FL2DCubismShader(Initializer)
    {
    }
};

class L2DCubismPixelNormalPremult : public FL2DCubismShader
{
    DECLARE_SHADER_TYPE(L2DCubismPixelNormalPremult, Global);
public:
    L2DCubismPixelNormalPremult() {}

    L2DCubismPixelNormalPremult(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FL2DCubismShader(Initializer)
    {
    }
};


class L2DCubismPixelMaskedInvertedPremult : public FL2DCubismShader
{
    DECLARE_SHADER_TYPE(L2DCubismPixelMaskedInvertedPremult, Global);
public:
    L2DCubismPixelMaskedInvertedPremult() {}

    L2DCubismPixelMaskedInvertedPremult(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FL2DCubismShader(Initializer)
    {
    }
};

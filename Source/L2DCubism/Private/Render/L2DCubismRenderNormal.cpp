// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubismRenderNormal.h"

#include "Containers/DynamicRHIResourceArray.h"
#include "RHI.h"

#include "GlobalShader.h"
#include "PipelineStateCache.h"
#include "RenderGraphResources.h"
#include "RHIStaticStates.h"
#include "CubismFramework.hpp"
#include "ImageUtils.h"
#include "Model/CubismModel.hpp"

#include "RHICommandList.h"
#include "RHIDefinitions.h"
#include "RHIResources.h"
#include "TextureResource.h"
#include "Shader.h"
#include "Engine/Texture2D.h"
#include "L2DCubismClippingManager.h"
#include "RenderUtils.h"
#include "L2DCubismAsset.h"
#include "L2DCubismRender.h"

#include "L2DCubismShader.h"

using namespace Core;
using namespace Framework;
using namespace Rendering;

struct FCubismVertex
{
    FVector2D Position;
    FVector2D UV;

    FCubismVertex(float x, float y, float z, float w)
        : Position(x, y)
          , UV(z, w)
    {
    }
};

class FCubismVertexDeclaration : public FRenderResource
{
public:
    FVertexDeclarationRHIRef VertexDeclarationRHI;

    void InitRHI() override
    {
        FVertexDeclarationElementList Elements;
        uint32 Stride = sizeof(FCubismVertex);
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FCubismVertex, Position), VET_Float2, 0, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FCubismVertex, UV), VET_Float2, 1, Stride));
        VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
    }

    void ReleaseRHI() override
    {
        VertexDeclarationRHI.SafeRelease();
    }
};

TGlobalResource<FCubismVertexDeclaration> GCubismVertexDeclaration;

void FCubismVertexBuffer::InitRHI()
{
    // create a static vertex buffer
    FRHIResourceCreateInfo CreateInfo;
    VertexBufferRHI = RHICreateVertexBuffer(sizeof(FCubismVertex) * 4, BUF_Static, CreateInfo);
    void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, sizeof(FCubismVertex) * 4, RLM_WriteOnly);
    static const FCubismVertex Vertices[4] =
    {
        FCubismVertex(-0.9, -0.9, 0, 0),
        FCubismVertex(-0.9, +0.9, 0, 1),
        FCubismVertex(+0.9, -0.9, 1, 0),
        FCubismVertex(+0.9, +0.9, 1, 1),
    };
    FMemory::Memcpy(VoidPtr, Vertices, sizeof(FCubismVertex) * 4);
    RHIUnlockVertexBuffer(VertexBufferRHI);
}

TGlobalResource<FCubismVertexBuffer> GCubismVertexScreenBuffer;


L2DCubismRenderNormal::L2DCubismRenderNormal()
    : ClippingManager(nullptr)
      , ClippingContextBufferForMask(nullptr)
      , ClippingContextBufferForDraw(nullptr)
      , MaskTexture(nullptr)
{
}

L2DCubismRenderNormal::~L2DCubismRenderNormal()
{

    ReleaseBuffers();
}

CubismRenderer* L2DCubismRenderNormal::Create()
{
    return CSM_NEW L2DCubismRenderNormal();
}

void L2DCubismRenderNormal::CreateBuffers(CubismModel* csmModel)
{
    SortedDrawableIndexList.SetNum(csmModel->GetDrawableCount());
    int DrawableCount = csmModel->GetDrawableCount();

    ENQUEUE_RENDER_COMMAND(CubismGetIndex)(
        [=](FRHICommandListImmediate& RHICmdList)
        {
            checkSlow(IsInRenderingThread());

            for (int i = 0; i < DrawableCount; i++)
            {
                // ���_���W�̏�񂪐����ύX������
                const csmInt32 VertexCount = csmModel->GetDrawableVertexCount(i);
                if (0 < VertexCount)
                {
                    FRHIResourceCreateInfo CreateInfoVertex;
                    FVertexBufferRHIRef p = RHICreateVertexBuffer(VertexCount * sizeof(FCubismVertex), BUF_Dynamic,
                                                                  CreateInfoVertex);
                    VertexBuffers.Add(i, p);
                }

                // ���_�C���f�b�N�X�͌Œ�
                const csmInt32 IndexCount = csmModel->GetDrawableVertexIndexCount(i);
                if (0 < IndexCount)
                {
                    const csmUint16* Indexes = const_cast<csmUint16*>(csmModel->GetDrawableVertexIndices(i));

                    FRHIResourceCreateInfo CreateInfoIndex;
                    FIndexBufferRHIRef p = RHICreateIndexBuffer(sizeof(csmUint16), sizeof(csmUint16) * IndexCount,
                                                                BUF_Static, CreateInfoIndex);
                    void* LockIndexBuffer = RHILockIndexBuffer(p, 0, sizeof(csmUint16) * IndexCount, RLM_WriteOnly);
                    FMemory::Memcpy(LockIndexBuffer, Indexes, sizeof(csmUint16) * IndexCount);
                    RHIUnlockIndexBuffer(p);

                    IndexBuffers.Add(i, p);
                }
            }
        });
}

void L2DCubismRenderNormal::Initialize(CubismModel* csmModel)
{
    CreateBuffers(csmModel);

    if (csmModel->IsUsingMasking())
    {
        ClippingManager = MakeShared<L2DCubismClippingManager>();
        ClippingManager->Initialize(*csmModel);
    }

    CubismRenderer::Initialize(csmModel);

    {
        const csmInt32 bufferHeight = ClippingManager->GetClippingMaskBufferSize();

        uint32 Flags = 0;
        Flags |= TexCreate_RenderTargetable;
        Flags |= TexCreate_ShaderResource;
        FRHIResourceCreateInfo CreateInfo;
        // 1�������i�`����Ȃ��j�̈�A0���L���i�`�����j�̈�B�i�V�F�[�_�� Cd*Cs��0�ɋ߂��l�������ă}�X�N�����B1��������Ɖ����N����Ȃ��j
        CreateInfo.ClearValueBinding = FClearValueBinding(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
        MaskTexture = RHICreateTexture2D(bufferHeight, bufferHeight, PF_B8G8R8A8, 1, 1, Flags,
                                         CreateInfo);
    }
}

// �`���̎w��
void L2DCubismRenderNormal::SetTextureRenderTarget2D(UTextureRenderTarget2D* p)
{
    TextureRenderTarget2D = p;
}

// �`���̎擾
UTextureRenderTarget2D* L2DCubismRenderNormal::GetTextureRenderTarget2D() const
{
    return TextureRenderTarget2D;
}

void L2DCubismRenderNormal::BindTexture(csmUint32 modelTextureAssign, UTexture2D* textureView)
{
    if (Textures.Contains(modelTextureAssign))
    {
        Textures[modelTextureAssign] = textureView;
    }
    else
    {
        Textures.Add(modelTextureAssign, textureView);
    }
}

void L2DCubismRenderNormal::ReleaseBuffers()
{
    for (auto& Elem : VertexBuffers)
    {
        Elem.Value.SafeRelease();
        Elem.Value = nullptr;
    }
    VertexBuffers.Empty();

    for (auto& Elem : IndexBuffers)
    {
        Elem.Value.SafeRelease();
        Elem.Value = nullptr;
    }
    IndexBuffers.Empty();

    if (MaskTexture != nullptr)
    {
        MaskTexture.SafeRelease();
    }

    if (VertexBufferRHI != nullptr)
    {
        VertexBufferRHI.SafeRelease();
        VertexBufferRHI = nullptr;
    }
    if (IndexBufferRHI != nullptr)
    {
        IndexBufferRHI.SafeRelease();
        IndexBufferRHI = nullptr;
    }

    ClippingManager = nullptr;
}

void L2DCubismRenderNormal::DoDrawModel()
{
    const csmInt32* RenderOrder = GetModel()->GetDrawableRenderOrders();
    const csmInt32 DrawableCount = GetModel()->GetDrawableCount();

    // �`�揇�Ƀ\�[�g
    for (csmInt32 i = 0; i < DrawableCount; ++i)
    {
        const csmInt32 order = RenderOrder[i];
        SortedDrawableIndexList[order] = i;
    }

    ENQUEUE_RENDER_COMMAND(L2DCubismDrawCommand)([=]
    (FRHICommandListImmediate& RHICmdList)
        {
            check(IsInRenderingThread());

            // �����_�����O��e�N�X�`���[
            FTextureRenderTargetResource* OutTextureRenderTargetResource = GetTextureRenderTarget2D()->
                GetRenderTargetResource();

            // �����_�����O��e�N�X�`���[���N���A�[
            {
                FRHITexture2D* RenderTargetTexture = OutTextureRenderTargetResource->GetRenderTargetTexture();
                RHICmdList.TransitionResource(EResourceTransitionAccess::EWritable, RenderTargetTexture);

                FRHIRenderPassInfo RPInfo(RenderTargetTexture, ERenderTargetActions::Clear_Store,
                                          OutTextureRenderTargetResource->TextureRHI);
                RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawClearForTarget"));
                RHICmdList.EndRenderPass();
            }

            // �}�X�N�p�e�N�X�`��
            FRHITexture2D* OutMaskTextureTarget = MaskTexture;
            // �}�X�N�p�e�N�X�`���[���N���A�[
            {
                RHICmdList.TransitionResource(EResourceTransitionAccess::EWritable, OutMaskTextureTarget);

                FRHIRenderPassInfo RPInfo(OutMaskTextureTarget,
                                          ERenderTargetActions::Clear_Store, OutMaskTextureTarget);

                RHICmdList.BeginRenderPass(RPInfo, TEXT("DrawClearForMask"));
                RHICmdList.EndRenderPass();
            }

            //------------ �N���b�s���O�}�X�N�E�o�b�t�@�O���������̏ꍇ ------------
            if (ClippingManager != nullptr)
            {
                ClippingManager->SetupClippingContext(RHICmdList, *GetModel(), this);
            }

            const bool bDebugMaskTexture = false;
            if (!bDebugMaskTexture)
            {
                //�@�`��
                for (csmInt32 j = 0; j < DrawableCount; ++j)
                {
                    const csmInt32 DrawableIndex = SortedDrawableIndexList[j];

                    // Drawable���\����ԂłȂ���Ώ������p�X����
                    if (!GetModel()->GetDrawableDynamicFlagIsVisible(DrawableIndex))
                    {
                        continue;
                    }

                    // �N���b�s���O�}�X�N���Z�b�g����
                    L2DCubismClippingContext* clipContext = (ClippingManager != nullptr)
                                                                ? (*ClippingManager->GetClippingContextListForDraw())[
                                                                    DrawableIndex]
                                                                : nullptr;

                    if (clipContext != nullptr && IsUsingHighPrecisionMask() && clipContext->bUsing) // �}�X�N�������K�v������
                    {
                        const int clipDrawCount = clipContext->ClippingIdCount;
                        for (int ctx = 0; ctx < clipDrawCount; ctx++)
                        {
                            const int clipDrawIndex = clipContext->ClippingIdList[ctx];
                    
                            // ���_��񂪍X�V����Ă��炸�A�M�������Ȃ��ꍇ�͕`����p�X����
                            if (!GetModel()->GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
                            {
                                continue;
                            }
                    
                            IsCulling(GetModel()->GetDrawableCulling(clipDrawIndex) != 0);
                    
                            // �����p�̕ϊ���K�p���ĕ`��
                            // �`�����l�����؂�ւ���K�v������(A,R,G,B)
                            SetClippingContextBufferForMask(clipContext);
                    
                            DrawMeshRHI(
                                RHICmdList,
                                OutTextureRenderTargetResource,
                                GetModel()->GetDrawableTextureIndices(clipDrawIndex),
                                clipDrawIndex,
                                GetModel()->GetDrawableVertexIndexCount(clipDrawIndex),
                                GetModel()->GetDrawableVertexCount(clipDrawIndex),
                                const_cast<csmUint16*>(GetModel()->GetDrawableVertexIndices(clipDrawIndex)),
                                const_cast<csmFloat32*>(GetModel()->GetDrawableVertices(clipDrawIndex)),
                                reinterpret_cast<csmFloat32*>(const_cast<csmVector2*>(GetModel()->
                                    GetDrawableVertexUvs(clipDrawIndex))),
                                GetModel()->GetDrawableOpacity(clipDrawIndex),
                                CubismBlendMode_Normal, //�N���b�s���O�͒ʏ�`�������
                                false // �}�X�N�������̓N���b�s���O�̔��]�g�p�͑S���֌W���Ȃ�
                            );
                        }
                        SetClippingContextBufferForMask(nullptr);
                    }
                    
                    // �N���b�s���O�}�X�N���Z�b�g����
                    SetClippingContextBufferForDraw(clipContext);

                    IsCulling(GetModel()->GetDrawableCulling(DrawableIndex) != 0);

                    DrawMeshRHI(
                        RHICmdList,
                        OutTextureRenderTargetResource,
                        GetModel()->GetDrawableTextureIndices(DrawableIndex),
                        DrawableIndex,
                        GetModel()->GetDrawableVertexIndexCount(DrawableIndex),
                        GetModel()->GetDrawableVertexCount(DrawableIndex),
                        const_cast<csmUint16*>(GetModel()->GetDrawableVertexIndices(DrawableIndex)),
                        const_cast<csmFloat32*>(GetModel()->GetDrawableVertices(DrawableIndex)),
                        reinterpret_cast<csmFloat32*>(const_cast<csmVector2*>(GetModel()->GetDrawableVertexUvs(
                            DrawableIndex))),
                        GetModel()->GetDrawableOpacity(DrawableIndex),
                        GetModel()->GetDrawableBlendMode(DrawableIndex),
                        GetModel()->GetDrawableInvertedMask(DrawableIndex)
                    );
                }
            }
            else
            {
                // MaskTexture��Debug�̂��߂ɕ\������
                DrawTextureForDebug(
                    RHICmdList,
                    OutTextureRenderTargetResource
                );
            }
        });
}

void L2DCubismRenderNormal::DrawMask(FRHICommandListImmediate& RHICmdList, csmInt32 indexCount, csmInt32 vertexCount,
                                     csmUint16* indexArray, csmFloat32* vertexArray, csmFloat32* uvArray,
                                     FVertexBufferRHIRef VertexBuffer, FIndexBufferRHIRef IndexBuffer,
                                     FTextureRHIRef TextureRHI, FRHITexture2D* OutTextureRenderTargetMask)
{
    FRHIRenderPassInfo RPInfo(OutTextureRenderTargetMask,
                              ERenderTargetActions::DontLoad_Store, OutTextureRenderTargetMask);

    RHICmdList.BeginRenderPass(RPInfo, TEXT("L2DCubismCreateMask"));
    {
        // Get shaders.
        TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
        TShaderMapRef<L2DCubismVertSetupMask> VertexShader(GlobalShaderMap);
        TShaderMapRef<L2DCubismPixelSetupMask> PixelShader(GlobalShaderMap);
        // Set the graphic pipeline state.

        // �`�����l��
        const csmInt32 channelNo = GetClippingContextBufferForMask()->LayoutChannelNo;
        // �`�����l����RGBA�ɕϊ�
        FLinearColor* colorChannel = GetClippingContextBufferForMask()
                                     ->GetClippingManager()->GetChannelFlagAsColor(channelNo);

        FMatrix InProjectMatrix;
        FVector4 InChannelFlag;
        FVector4 InBaseColor;

        // �萔�o�b�t�@
        {
            csmRectF* rect = GetClippingContextBufferForMask()->LayoutBounds;

            InProjectMatrix = L2DCubismRender::ConvertToFMatrix(GetClippingContextBufferForMask()->MatrixForMask);
            InBaseColor = FVector4(rect->X * 2.0f - 1.0f, rect->Y * 2.0f - 1.0f, rect->GetRight() * 2.0f - 1.0f,
                                   rect->GetBottom() * 2.0f - 1.0f);
            InChannelFlag = FVector4(colorChannel->R, colorChannel->G, colorChannel->B, colorChannel->A);
        }

        /*
         * ���_�����������iGPU�j�փR�s�[
         */
        FCubismVertex* VertexBufferData = static_cast<FCubismVertex*>(RHICmdList.LockVertexBuffer(
            VertexBuffer, 0, vertexCount * sizeof(FCubismVertex), RLM_WriteOnly));

        for (int i = 0; i < vertexCount; i++)
        {
            VertexBufferData[i].Position = FVector2D(vertexArray[2 * i], vertexArray[2 * i + 1]);
            VertexBufferData[i].UV = FVector2D(uvArray[2 * i], uvArray[2 * i + 1]);
        }
        RHICmdList.UnlockVertexBuffer(VertexBuffer);

        /*
         * �C���f�b�N�X�����������iGPU�j�փR�s�[
         */
        // csmUint16* IndexBufferData = static_cast<csmUint16*>(RHICmdList.LockIndexBuffer(
        //  IndexBuffer, 0, indexCount * sizeof(csmUint16), RLM_WriteOnly));
        // for (int i = 0; i < indexCount; i++)
        // {
        //  IndexBufferData[i] = indexArray[i];
        // }
        // RHICmdList.UnlockIndexBuffer(IndexBuffer);

        /* ----------- */

        /*
         * �V�F�[�_�[�փp�����[�^��˂�����
         */
        FGraphicsPipelineStateInitializer GraphicsPSOInit;
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
        GraphicsPSOInit.BlendState = TStaticBlendState<
            CW_RGBA, BO_Add, BF_Zero, BF_InverseSourceColor, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI();
        GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
        GraphicsPSOInit.PrimitiveType = PT_TriangleList;
        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GCubismVertexDeclaration.VertexDeclarationRHI;

        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
        GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);

        VertexShader->SetParameters(RHICmdList, VertexShader->GetVertexShader(), InProjectMatrix,
                                    InBaseColor, InChannelFlag, TextureRHI);

        PixelShader->SetParameters(RHICmdList, PixelShader->GetPixelShader(), InProjectMatrix,
                                   InBaseColor, InChannelFlag, TextureRHI);


        // Update viewport.
        float MaskSize = GetClippingContextBufferForMask()->GetClippingManager()->ClippingMaskBufferSize;
        RHICmdList.SetViewport(
            0, 0, 0.f,
            MaskSize, MaskSize, 1.f);

        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        // Set VertextBuffer
        RHICmdList.SetStreamSource(0, VertexBuffer, 0);
        // Draw Primitives
        RHICmdList.DrawIndexedPrimitive(
            IndexBuffer,
            0,
            0,
            vertexCount,
            0,
            indexCount / 3,
            1);
    }
    RHICmdList.EndRenderPass();
}

void L2DCubismRenderNormal::DrawTarget(FRHICommandListImmediate& RHICmdList,
                                       FTextureRenderTargetResource* OutTextureRenderTargetResource,
                                       csmInt32 indexCount, csmInt32 vertexCount, 
                                       csmFloat32* vertexArray, csmFloat32* uvArray, csmFloat32 opacity,
                                       CubismTextureColor& modelColorRGBA, CubismBlendMode colorBlendMode,
                                       csmBool invertedMask, FVertexBufferRHIRef VertexBuffer,
                                       FIndexBufferRHIRef IndexBuffer, FTextureRHIRef TextureRHI)
{
    FRHITexture2D* RenderTargetTexture = OutTextureRenderTargetResource->GetRenderTargetTexture();
    RHICmdList.TransitionResource(EResourceTransitionAccess::EWritable, RenderTargetTexture);
    
    FRHIRenderPassInfo RPInfo(RenderTargetTexture,
                              ERenderTargetActions::Load_Store, OutTextureRenderTargetResource->TextureRHI);

    RHICmdList.BeginRenderPass(RPInfo, TEXT("L2DCubismRenderPass"));
    {
        // Set the graphic pipeline state.

        FVector4 InBaseColor = FVector4(1.0f, 1.0f, 1.0f, opacity);

        /*
         * ���_�����������iGPU�j�փR�s�[
         */
        FCubismVertex* VertexBufferData = static_cast<FCubismVertex*>(RHICmdList.LockVertexBuffer(
            VertexBuffer, 0, vertexCount * sizeof(FCubismVertex), RLM_WriteOnly));

        for (csmInt32 i = 0; i < vertexCount; i++)
        {
            VertexBufferData[i].Position = FVector2D(vertexArray[2 * i], vertexArray[2 * i + 1]);
            VertexBufferData[i].UV = FVector2D(uvArray[2 * i], uvArray[2 * i + 1]);
        }
        RHICmdList.UnlockVertexBuffer(VertexBuffer);

        /*
         * �C���f�b�N�X�����������iGPU�j�փR�s�[
         */
        // csmUint16* IndexBufferData = static_cast<csmUint16*>(RHICmdList.LockIndexBuffer(
        //  IndexBuffer, 0, indexCount * sizeof(csmUint16), RLM_WriteOnly));
        // for (csmInt32 i = 0; i < indexCount; i++)
        // {
        //  IndexBufferData[i] = indexArray[i];
        // }
        // RHICmdList.UnlockIndexBuffer(IndexBuffer);

        /*
         * �V�F�[�_�[�փp�����[�^��˂�����
         */

        FMatrix InProjectMatrix = FMatrix::Identity;
        FMatrix InClipMatrix = FMatrix::Identity;
        FVector4 InChannelFlag = FVector4(1, 1, 1, 1);

        const csmBool masked = GetClippingContextBufferForDraw() != nullptr; // ���̕`��I�u�W�F�N�g�̓}�X�N�Ώۂ�
        const csmBool premult = IsPremultipliedAlpha();
        const csmInt32 offset = (masked ? (invertedMask ? 2 : 1) : 0) + (IsPremultipliedAlpha() ? 3 : 0);

        FGraphicsPipelineStateInitializer GraphicsPSOInit;
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

        // Get shaders.
        TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

        TShaderMapRef<L2DCubismVertNormal> VSNormal(GlobalShaderMap);
        TShaderMapRef<L2DCubismVertMasked> VSMasked(GlobalShaderMap);

        TShaderMapRef<L2DCubismPixelNormal> PSNormal(GlobalShaderMap);
        TShaderMapRef<L2DCubismPixelMasked> PSMasked(GlobalShaderMap);
        TShaderMapRef<L2DCubismPixelMaskedInverted> PSMaskedInverted(GlobalShaderMap);
        TShaderMapRef<L2DCubismPixelMaskedInvertedPremult> PSMaskedInvertedPremult(GlobalShaderMap);

        TShaderMapRef<L2DCubismPixelSetupMask> PSSetupMask(GlobalShaderMap);

        // �萔�o�b�t�@
        {
            if (masked)
            {
                // View���W��ClippingContext�̍��W�ɕϊ����邽�߂̍s���ݒ�

                InClipMatrix = L2DCubismRender::ConvertToFMatrix(GetClippingContextBufferForDraw()->MatrixForDraw);

                // �g�p����J���[�`�����l����ݒ�
                const csmInt32 channelNo = GetClippingContextBufferForDraw()->LayoutChannelNo;
                FLinearColor* colorChannel = GetClippingContextBufferForDraw()
                                             ->GetClippingManager()->GetChannelFlagAsColor(channelNo);
                InChannelFlag = FVector4(colorChannel->R, colorChannel->G, colorChannel->B, colorChannel->A);
            }

            // �v���W�F�N�V����Mtx
            CubismMatrix44 mvp = GetMvpMatrix();
            InProjectMatrix = L2DCubismRender::ConvertToFMatrix(mvp);
            // �F
            InBaseColor = FVector4(modelColorRGBA.R, modelColorRGBA.G, modelColorRGBA.B, modelColorRGBA.A);
        }

        // �u�����h�X�e�[�g
        switch (colorBlendMode)
        {
        case CubismBlendMode_Normal:
        default:
            GraphicsPSOInit.BlendState = TStaticBlendState<
                CW_RGBA, BO_Add, BF_One, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
            break;

        case CubismBlendMode_Additive:
            GraphicsPSOInit.BlendState = TStaticBlendState<
                CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_Zero, BF_One>::GetRHI();
            break;

        case CubismBlendMode_Multiplicative:
            GraphicsPSOInit.BlendState = TStaticBlendState<
                CW_RGBA, BO_Add, BF_DestColor, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_One>::GetRHI();
            break;
        }

        // �V�F�[�_�Z�b�g
        if (masked)
        {
            if (premult)
            {
                if (invertedMask)
                {
                    GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSMasked);
                    GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(
                        *PSMaskedInvertedPremult);

                    VSMasked->SetParameters(RHICmdList, VSMasked->GetVertexShader(), InProjectMatrix,
                                            InClipMatrix, InBaseColor, InChannelFlag, TextureRHI,
                                            MaskTexture->GetTexture2D());

                    PSMaskedInvertedPremult->SetParameters(RHICmdList, PSMaskedInvertedPremult->GetPixelShader(),
                                                           InProjectMatrix, InClipMatrix,
                                                           InBaseColor, InChannelFlag, TextureRHI,
                                                           MaskTexture->GetTexture2D());
                }
                else
                {
                    GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSMasked);
                    GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PSMaskedInverted);

                    VSMasked->SetParameters(RHICmdList, VSMasked->GetVertexShader(), InProjectMatrix,
                                            InClipMatrix, InBaseColor, InChannelFlag, TextureRHI,
                                            MaskTexture->GetTexture2D());

                    PSMaskedInverted->SetParameters(RHICmdList, PSMaskedInverted->GetPixelShader(), InProjectMatrix,
                                                    InClipMatrix,
                                                    InBaseColor, InChannelFlag, TextureRHI,
                                                    MaskTexture->GetTexture2D());
                }
            }
            else
            {
                if (invertedMask)
                {
                    GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSMasked);
                    GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PSMaskedInverted);

                    VSMasked->SetParameters(RHICmdList, VSMasked->GetVertexShader(), InProjectMatrix,
                                            InClipMatrix, InBaseColor, InChannelFlag, TextureRHI,
                                            MaskTexture->GetTexture2D());

                    PSMaskedInverted->SetParameters(RHICmdList, PSMaskedInverted->GetPixelShader(), InProjectMatrix,
                                                    InClipMatrix,
                                                    InBaseColor, InChannelFlag, TextureRHI,
                                                    MaskTexture->GetTexture2D());
                }
                else
                {
                    GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSMasked);
                    GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PSMasked);

                    VSMasked->SetParameters(RHICmdList, VSMasked->GetVertexShader(), InProjectMatrix,
                                            InClipMatrix, InBaseColor, InChannelFlag, TextureRHI,
                                            MaskTexture->GetTexture2D());

                    PSMasked->SetParameters(RHICmdList, PSMasked->GetPixelShader(), InProjectMatrix, InClipMatrix,
                                            InBaseColor, InChannelFlag, TextureRHI, MaskTexture->GetTexture2D());
                }
            }
        }
        else
        {
            if (premult)
            {
                GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSNormal);
                GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PSMaskedInvertedPremult);

                VSNormal->SetParameters(RHICmdList, VSNormal->GetVertexShader(), InProjectMatrix,
                                        InClipMatrix, InBaseColor, InChannelFlag, TextureRHI,
                                        MaskTexture->GetTexture2D());

                PSMaskedInvertedPremult->SetParameters(RHICmdList, PSMaskedInvertedPremult->GetPixelShader(),
                                                       InProjectMatrix, InClipMatrix,
                                                       InBaseColor, InChannelFlag, TextureRHI,
                                                       MaskTexture->GetTexture2D());
            }
            else
            {
                GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSNormal);
                GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PSNormal);

                VSNormal->SetParameters(RHICmdList, VSNormal->GetVertexShader(), InProjectMatrix,
                                        InClipMatrix, InBaseColor, InChannelFlag, TextureRHI,
                                        MaskTexture->GetTexture2D());

                PSNormal->SetParameters(RHICmdList, PSNormal->GetPixelShader(), InProjectMatrix, InClipMatrix,
                                        InBaseColor, InChannelFlag, TextureRHI, MaskTexture->GetTexture2D());
            }
        }

        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

        GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
        GraphicsPSOInit.PrimitiveType = PT_TriangleList;
        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GCubismVertexDeclaration.VertexDeclarationRHI;

        // Update viewport.
        RHICmdList.SetViewport(
            0, 0, 0.f,
            OutTextureRenderTargetResource->GetSizeX(), OutTextureRenderTargetResource->GetSizeY(), 1.f);

        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        // Set VertextBuffer
        RHICmdList.SetStreamSource(0, VertexBuffer, 0);
        // Draw Primitives
        RHICmdList.DrawIndexedPrimitive(IndexBuffer, 0, 0, vertexCount, 0, indexCount / 3, 1);
    }
    RHICmdList.EndRenderPass();
}

void L2DCubismRenderNormal::ExecuteDraw(FRHICommandListImmediate& RHICmdList,
                                        FTextureRenderTargetResource* OutTextureRenderTargetResource,
                                        csmInt32 textureNo, csmInt32 drawableIndex, csmInt32 indexCount,
                                        csmInt32 vertexCount, csmUint16* indexArray, csmFloat32* vertexArray,
                                        csmFloat32* uvArray, csmFloat32 opacity, CubismTextureColor& modelColorRGBA,
                                        CubismBlendMode colorBlendMode, csmBool invertedMask)
{
    FVertexBufferRHIRef VertexBuffer = VertexBuffers[drawableIndex];
    FIndexBufferRHIRef IndexBuffer = IndexBuffers[drawableIndex];

    FTextureRHIRef TextureRHI = Textures[textureNo]->Resource->TextureRHI;

    if (GetClippingContextBufferForMask() != nullptr) // �}�X�N������
    {
        FRHITexture2D* OutTextureRenderTargetMask = MaskTexture;

        DrawMask(RHICmdList,
                 indexCount, vertexCount, indexArray, vertexArray, uvArray, VertexBuffer, IndexBuffer, TextureRHI,
                 OutTextureRenderTargetMask);
    }
    else // �}�X�N�����ȊO�̏ꍇ
    {
        DrawTarget(RHICmdList, OutTextureRenderTargetResource, indexCount, vertexCount, vertexArray,
                   uvArray,
                   opacity, modelColorRGBA, colorBlendMode, invertedMask, VertexBuffer, IndexBuffer, TextureRHI);
    }
}

void L2DCubismRenderNormal::DrawMeshRHI(FRHICommandListImmediate& RHICmdList,
                                        FTextureRenderTargetResource* OutTextureRenderTargetResource,
                                        csmInt32 textureNo, csmInt32 drawableIndex, csmInt32 indexCount,
                                        csmInt32 vertexCount
                                        , csmUint16* indexArray, csmFloat32* vertexArray, csmFloat32* uvArray
                                        , csmFloat32 opacity, CubismBlendMode colorBlendMode, csmBool invertedMask)
{
    check(IsInRenderingThread());

    if (indexCount == 0)
    {
        // �`�敨����
        return;
    }
    // �`��s�v�Ȃ�`�揈�����X�L�b�v����
    if (opacity <= 0.0f && GetClippingContextBufferForMask() == nullptr)
    {
        return;
    }

    CubismTextureColor modelColorRGBA = GetModelColor();

    if (GetClippingContextBufferForMask() == nullptr) // �}�X�N�������ȊO
    {
        modelColorRGBA.A *= opacity;
        if (IsPremultipliedAlpha())
        {
            modelColorRGBA.R *= modelColorRGBA.A;
            modelColorRGBA.G *= modelColorRGBA.A;
            modelColorRGBA.B *= modelColorRGBA.A;
        }
    }

    ExecuteDraw(RHICmdList, OutTextureRenderTargetResource, textureNo, drawableIndex, indexCount, vertexCount,
                indexArray,
                vertexArray, uvArray, opacity, modelColorRGBA, colorBlendMode, invertedMask);

    SetClippingContextBufferForDraw(nullptr);
    SetClippingContextBufferForMask(nullptr);
}

CubismRenderer* CubismRenderer::Create()
{
    return CSM_NEW L2DCubismRenderNormal();
}

void CubismRenderer::StaticRelease()
{
}

void L2DCubismRenderNormal::SetClippingContextBufferForDraw(L2DCubismClippingContext* clip)
{
    ClippingContextBufferForDraw = clip;
}

L2DCubismClippingContext* L2DCubismRenderNormal::GetClippingContextBufferForDraw() const
{
    return ClippingContextBufferForDraw;
}

void L2DCubismRenderNormal::SetClippingContextBufferForMask(L2DCubismClippingContext* clip)
{
    ClippingContextBufferForMask = clip;
}

L2DCubismClippingContext* L2DCubismRenderNormal::GetClippingContextBufferForMask() const
{
    return ClippingContextBufferForMask;
}

csmInt32 L2DCubismRenderNormal::GetClippingMaskBufferSize() const
{
    return ClippingManager->GetClippingMaskBufferSize();
}


void L2DCubismRenderNormal::DrawTextureForDebug(
    FRHICommandListImmediate& RHICmdList,
    FTextureRenderTargetResource* OutTextureRenderTargetResource)
{
    FRHIRenderPassInfo RPInfo(OutTextureRenderTargetResource->GetRenderTargetTexture(),
                              ERenderTargetActions::Clear_Store, OutTextureRenderTargetResource->TextureRHI);

    RHICmdList.BeginRenderPass(RPInfo, TEXT("L2DCubismRenderTextureDebug"));
    {
        // Set the graphic pipeline state.

        FVector4 InBaseColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);

        /*
         * �V�F�[�_�[�փp�����[�^��˂�����
         */

        FMatrix InProjectMatrix;
        FMatrix InClipMatrix;
        FVector4 InChannelFlag = FVector4(1, 1, 1, 1);
        InProjectMatrix.SetIdentity();
        InClipMatrix.SetIdentity();

        FGraphicsPipelineStateInitializer GraphicsPSOInit;
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

        // Get shaders.
        TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

        TShaderMapRef<L2DCubismVertNormal> VSNormal(GlobalShaderMap);
        TShaderMapRef<L2DCubismPixelNormal> PSNormal(GlobalShaderMap);

        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VSNormal);
        GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PSNormal);

        VSNormal->SetParameters(RHICmdList, VSNormal->GetVertexShader(), InProjectMatrix,
                                InClipMatrix, InBaseColor, InChannelFlag, MaskTexture->GetTexture2D(),
                                MaskTexture->GetTexture2D());

        PSNormal->SetParameters(RHICmdList, PSNormal->GetPixelShader(), InProjectMatrix, InClipMatrix,
                                InBaseColor, InChannelFlag, MaskTexture->GetTexture2D(), MaskTexture->GetTexture2D());

        GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
        GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
        GraphicsPSOInit.PrimitiveType = PT_TriangleList;
        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GCubismVertexDeclaration.VertexDeclarationRHI;

        // Update viewport.
        RHICmdList.SetViewport(
            0, 0, 0.f,
            OutTextureRenderTargetResource->GetSizeX(), OutTextureRenderTargetResource->GetSizeY(), 1.f);

        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        // Set VertextBuffer
        RHICmdList.SetStreamSource(0, GCubismVertexScreenBuffer.VertexBufferRHI, 0);
        // Draw Primitives
        RHICmdList.DrawIndexedPrimitive(GTwoTrianglesIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
    }
    RHICmdList.EndRenderPass();
}

void L2DCubismRenderNormal::DrawMesh(csmInt32 textureNo, csmInt32 indexCount, csmInt32 vertexCount
                                     , csmUint16* indexArray, csmFloat32* vertexArray, csmFloat32* uvArray
                                     , csmFloat32 opacity, CubismBlendMode colorBlendMode, csmBool invertedMask)
{
    UE_LOG(LogL2DCubism, Log, TEXT("Use 'DrawMeshRHI' function"));
    CSM_ASSERT(0);
}

void L2DCubismRenderNormal::SaveProfile()
{
};

void L2DCubismRenderNormal::RestoreProfile()
{
}

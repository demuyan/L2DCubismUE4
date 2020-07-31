// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "L2DCubismClippingManager.h"
#include "Model/CubismModel.hpp"
#include "L2DCubismShader.h"
#include "RHIResources.h"

/**
 * 
 */

using namespace Live2D::Cubism::Framework;


class FCubismIndexBufferNew : public FIndexBuffer
{
public:
    virtual void InitRHI() override
    {
        FRHIResourceCreateInfo CreateInfo;
        IndexBufferRHI = RHICreateIndexBuffer(sizeof(int16), uint16(NumIndices) * sizeof(int16), BUF_Static,
                                              CreateInfo);
    }

    int32 NumIndices;
};

/** Test Texture Draw */
class FCubismVertexBuffer : public FVertexBuffer
{
public:
    /**
    * Initialize the RHI for this rendering resource
    */
    virtual void InitRHI() override;
};


class L2DCubismClippingContext;

class L2DCubismRenderNormal : public Rendering::CubismRenderer
{
    friend class Rendering::CubismRenderer;
    friend class CubismClippingManager;

public:
    TMap<int32, FVertexBufferRHIRef> VertexBuffers;
    TMap<int32, FIndexBufferRHIRef> IndexBuffers;

    TMap<int32, UTexture2D*> Textures;

    FTexture2DRHIRef MaskTexture;

    L2DCubismRenderNormal();
    ~L2DCubismRenderNormal();
    Rendering::CubismRenderer* Create();
    void CreateBuffers(CubismModel* csmModel);
    void SetClippingMaskBufferSize(csmInt32 size);
    csmInt32 GetClippingMaskBufferSize() const;

    void DrawTextureForDebug(FRHICommandListImmediate& RHICmdList,
                             FTextureRenderTargetResource* OutTextureRenderTargetResource);

    void InitializeConstantParameters();
    virtual void Initialize(CubismModel* csmModel);
    void ReleaseBuffers();
    void GetTexture(CubismModel* csmModel);
    void InitRHI(CubismModel* csmModel);

    void GetVertexBuffer();
    void GetIndexBuffer();
    void GetTexture();
    void Rendering(CubismModel* csmModel, UTextureRenderTarget2D* RenderTarget2D);

    /**
     * @brief   ���f����`�悷����ۂ̏���
    *
    */
    virtual void DoDrawModel() override;
    void DrawMask(FRHICommandListImmediate& RHICmdList, csmInt32 indexCount, csmInt32 vertexCount,
                  csmUint16* indexArray,
                  csmFloat32* vertexArray, csmFloat32* uvArray, FVertexBufferRHIRef VertexBuffer,
                  FIndexBufferRHIRef IndexBuffer, FTextureRHIRef TextureRHI, FRHITexture2D* OutTextureRenderTargetMask);
    void DrawTarget(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* OutTextureRenderTargetResource,
                    csmInt32 indexCount, csmInt32 vertexCount, csmFloat32* vertexArray,
                    csmFloat32* uvArray, csmFloat32 opacity, CubismRenderer::CubismTextureColor& modelColorRGBA,
                    CubismRenderer::CubismBlendMode colorBlendMode, csmBool invertedMask,
                    FVertexBufferRHIRef VertexBuffer,
                    FIndexBufferRHIRef IndexBuffer, FTextureRHIRef TextureRHI);
    void BindTexture(csmUint32 modelTextureAssign, UTexture2D* textureView);

    void SetTextureRenderTarget2D(UTextureRenderTarget2D* p);
    UTextureRenderTarget2D* GetTextureRenderTarget2D() const;
    FVertexBufferRHIRef GetVertexBuffer(csmInt32 vertexCount, csmFloat32* vertexArray, csmFloat32* uvArray);
    FIndexBufferRHIRef GetIndexBuffer(csmInt32 indexCount, csmUint16* indexArray);
    virtual void DrawMesh(csmInt32 textureNo, csmInt32 indexCount, csmInt32 vertexCount
                          , csmUint16* indexArray, csmFloat32* vertexArray, csmFloat32* uvArray
                          , csmFloat32 opacity, CubismBlendMode colorBlendMode, csmBool invertedMask) override;

    void DrawMeshRHI(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* OutTextureRenderTargetResource,
                     csmInt32 textureNo, csmInt32 drawableIndex, csmInt32 indexCount, csmInt32 vertexCount
                     , csmUint16* indexArray, csmFloat32* vertexArray, csmFloat32* uvArray
                     , csmFloat32 opacity, CubismBlendMode colorBlendMode, csmBool invertedMask);

    void SetClippingContextBufferForMask(L2DCubismClippingContext* clip);

private:

    void ExecuteDraw(FRHICommandListImmediate& RHICmdList, FTextureRenderTargetResource* OutTextureRenderTargetResource,
                     csmInt32 textureNo, csmInt32 drawableIndex, csmInt32 indexCount, csmInt32 vertexCount,
                     csmUint16* indexArray, csmFloat32* vertexArray, csmFloat32* uvArray, csmFloat32 opacity,
                     CubismTextureColor& modelColorRGBA, CubismBlendMode colorBlendMode, csmBool invertedMask);

    void CreateMask(FRHICommandListImmediate& RHICmdList,
                    csmInt32 textureNo, csmInt32 drawableIndex, csmInt32 indexCount, csmInt32 vertexCount,
                    csmUint16* indexArray, csmFloat32* vertexArray, csmFloat32* uvArray, csmFloat32 opacity);


    // Prevention of copy Constructor
    L2DCubismRenderNormal(const L2DCubismRenderNormal&);
    L2DCubismRenderNormal& operator=(const L2DCubismRenderNormal&);

    /**
     * @brief   ���f���`�撼�O�̃X�e�[�g��ێ�����
    */
    virtual void SaveProfile() override;

    /**
     * @brief   ���f���`�撼�O�̃X�e�[�g��ێ�����
    */
    virtual void RestoreProfile() override;

    /**
     * @brief   �}�X�N�e�N�X�`���ɕ`�悷��N���b�s���O�R���e�L�X�g���擾����B
     *
     * @return  �}�X�N�e�N�X�`���ɕ`�悷��N���b�s���O�R���e�L�X�g
     */
    L2DCubismClippingContext* GetClippingContextBufferForMask() const;

    /**
     * @brief   ��ʏ�ɕ`�悷��N���b�s���O�R���e�L�X�g���Z�b�g����B
     */
    void SetClippingContextBufferForDraw(L2DCubismClippingContext* clip);

    /**
     * @brief   ��ʏ�ɕ`�悷��N���b�s���O�R���e�L�X�g���擾����B
     *
     * @return  ��ʏ�ɕ`�悷��N���b�s���O�R���e�L�X�g
     */
    L2DCubismClippingContext* GetClippingContextBufferForDraw() const;

    FVertexBufferRHIRef VertexBufferRHI;
    FIndexBufferRHIRef IndexBufferRHI;

    TArray<csmInt32> SortedDrawableIndexList; ///< �`��I�u�W�F�N�g�̃C���f�b�N�X��`�揇�ɕ��ׂ����X�g
    TSharedPtr<class L2DCubismClippingManager> ClippingManager;

    UTextureRenderTarget2D* TextureRenderTarget2D;
    L2DCubismClippingContext* ClippingContextBufferForMask; ///< �}�X�N�e�N�X�`���ɕ`�悷�邽�߂̃N���b�s���O�R���e�L�X�g
    L2DCubismClippingContext* ClippingContextBufferForDraw; ///< ��ʏ�`�悷�邽�߂̃N���b�s���O�R���e�L�X�g
};

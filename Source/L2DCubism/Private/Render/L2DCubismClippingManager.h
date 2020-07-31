// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#pragma once

#include "Engine/TextureRenderTarget2D.h"
#include "CubismFramework.hpp"
#include "L2DCubismRenderNormal.h"
#include "Rendering/CubismRenderer.hpp"
#include "Type/csmRectF.hpp"
#include "Type/csmVector.hpp"

class L2DCubismClippingContext;
class L2DCubismRenderNormal;

using namespace Csm;
using namespace Live2D::Cubism::Framework::Rendering;

/**
 * 
 */
class L2DCubismClippingManager
{
    friend struct FCubismRender;
    friend class L2DCubismRenderNormal;

public:

    /**
     * @brief �J���[�`�����l��(RGBA)�̃t���O���擾����
     *
     * @param[in]   channelNo   ->   �J���[�`�����l��(RGBA)�̔ԍ�(0:R , 1:G , 2:B, 3:A)
     */
    FLinearColor* GetChannelFlagAsColor(csmInt32 channelNo);

    /**
     * @brief   �}�X�N�����`��I�u�W�F�N�g�Q�S�̂��͂ދ�`(���f�����W�n)���v�Z����
     *
     * @param[in]   model            ->  ���f���̃C���X�^���X
     * @param[in]   ClippingContext  ->  �N���b�s���O�}�X�N�̃R���e�L�X�g
     */
    void CalcClippedDrawTotalBounds(CubismModel& model, L2DCubismClippingContext* ClippingContext);

    L2DCubismClippingManager();
    virtual ~L2DCubismClippingManager();
    void Initialize(CubismModel& model);

    /**
     * @brief   �N���b�s���O�R���e�L�X�g���쐬����B���f���`�掞�Ɏ��s����B
     *
     * @param[in]   model       ->  ���f���̃C���X�^���X
     * @param[in]   renderer    ->  �����_���̃C���X�^���X
     */
    void SetupClippingContext(FRHICommandListImmediate& RHICmdList, CubismModel& model,
                              L2DCubismRenderNormal* renderer);

    /**
     * @brief   ���Ƀ}�X�N������Ă��邩���m�F�B<br>
     *          ����Ă���悤�ł���ΊY������N���b�s���O�}�X�N�̃C���X�^���X��Ԃ��B<br>
     *          ����Ă��Ȃ����NULL��Ԃ�
     *
     * @param[in]   drawableMasks    ->  �`��I�u�W�F�N�g���}�X�N����`��I�u�W�F�N�g�̃��X�g
     * @param[in]   drawableMaskCounts ->  �`��I�u�W�F�N�g���}�X�N����`��I�u�W�F�N�g�̐�
     * @return          �Y������N���b�s���O�}�X�N�����݂���΃C���X�^���X��Ԃ��A�Ȃ����NULL��Ԃ��B
     */
    L2DCubismClippingContext* FindSameClip(const csmInt32* drawableMasks, csmInt32 drawableMaskCounts) const;

    /**
     * @brief   �N���b�s���O�R���e�L�X�g��z�u���郌�C�A�E�g�B<br>
     *           �ЂƂ̃����_�[�e�N�X�`�����ɗ͂����ς��Ɏg���ă}�X�N�����C�A�E�g����B<br>
     *           �}�X�N�O���[�v�̐���4�ȉ��Ȃ�RGBA�e�`�����l���ɂP���}�X�N��z�u���A5�ȏ�6�ȉ��Ȃ�RGBA��2,2,1,1�Ɣz�u����B
     *
     * @param[in]   UsingClipCount  ->  �z�u����N���b�s���O�R���e�L�X�g�̐�
     */
    void SetupLayoutBounds(csmInt32 UsingClipCount) const;

    /**
     * @breif   �J���[�o�b�t�@�̃A�h���X���擾����
     *
     * @return  �J���[�o�b�t�@�̃A�h���X
     */
    // CubismOffscreenFrame_D3D11* GetColorBuffer() const;

    /**
     * @brief   ��ʕ`��Ɏg�p����N���b�s���O�}�X�N�̃��X�g���擾����
     *
     * @return  ��ʕ`��Ɏg�p����N���b�s���O�}�X�N�̃��X�g
     */
    csmVector<L2DCubismClippingContext*>* GetClippingContextListForDraw();

    /**
     *@brief  �N���b�s���O�}�X�N�o�b�t�@�̃T�C�Y��ݒ肷��
     *
     *@param  size -> �N���b�s���O�}�X�N�o�b�t�@�̃T�C�Y
     *
     */
    void SetClippingMaskBufferSize(csmInt32 size);

    /**
     *@brief  �N���b�s���O�}�X�N�o�b�t�@�̃T�C�Y���擾����
     *
     *@return �N���b�s���O�}�X�N�o�b�t�@�̃T�C�Y
     *
     */
    csmInt32 GetClippingMaskBufferSize() const;

    csmInt32 CurrentFrameNo; ///< �}�X�N�e�N�X�`���ɗ^����t���[���ԍ�

    TArray<FLinearColor*> ChannelColors;
    //  csmVector<Live2D::Cubism::Framework::Rendering::CubismRenderer::CubismTextureColor*>  ChannelColors;
    csmVector<L2DCubismClippingContext*> ClippingContextListForMask; ///< �}�X�N�p�N���b�s���O�R���e�L�X�g�̃��X�g
    csmVector<L2DCubismClippingContext*> ClippingContextListForDraw; ///< �`��p�N���b�s���O�R���e�L�X�g�̃��X�g
    csmInt32 ClippingMaskBufferSize; ///< �N���b�s���O�}�X�N�̃o�b�t�@�T�C�Y�i�����l:256�j
};


/**
 * @brief   �N���b�s���O�}�X�N�̃R���e�L�X�g
 */
class L2DCubismClippingContext
{
    friend class L2DCubismClippingManager;
    friend class CubismRenderer;

public:
    /**
     * @brief   �����t���R���X�g���N�^
     *
     */
    L2DCubismClippingContext(L2DCubismClippingManager* manager, const csmInt32* clippingDrawableIndices,
                             csmInt32 clipCount);

    /**
     * @brief   �f�X�g���N�^
     */
    virtual ~L2DCubismClippingContext();

    /**
     * @brief   ���̃}�X�N�ɃN���b�v�����`��I�u�W�F�N�g��ǉ�����
     *
     * @param[in]   drawableIndex   ->  �N���b�s���O�Ώۂɒǉ�����`��I�u�W�F�N�g�̃C���f�b�N�X
     */
    void AddClippedDrawable(csmInt32 drawableIndex);

    /**
     * @brief   ���̃}�X�N���Ǘ�����}�l�[�W���̃C���X�^���X���擾����B
     *
     * @return  �N���b�s���O�}�l�[�W���̃C���X�^���X
     */
    L2DCubismClippingManager* GetClippingManager();

    csmBool bUsing; ///< ���݂̕`���ԂŃ}�X�N�̏������K�v�Ȃ�true
    const csmInt32* ClippingIdList; ///< �N���b�s���O�}�X�N��ID���X�g
    csmInt32 ClippingIdCount; ///< �N���b�s���O�}�X�N�̐�
    csmInt32 LayoutChannelNo; ///< RGBA�̂�����̃`�����l���ɂ��̃N���b�v��z�u���邩(0:R , 1:G , 2:B , 3:A)
    csmRectF* LayoutBounds; ///< �}�X�N�p�`�����l���̂ǂ̗̈�Ƀ}�X�N�����邩(View���W-1..1, UV��0..1�ɒ���)
    csmRectF* AllClippedDrawRect; ///< ���̃N���b�s���O�ŁA�N���b�s���O�����S�Ă̕`��I�u�W�F�N�g�̈͂݋�`�i����X�V�j
    CubismMatrix44 MatrixForMask; ///< �}�X�N�̈ʒu�v�Z���ʂ�ێ�����s��
    CubismMatrix44 MatrixForDraw; ///< �`��I�u�W�F�N�g�̈ʒu�v�Z���ʂ�ێ�����s��
    csmVector<csmInt32>* ClippedDrawableIndexList; ///< ���̃}�X�N�ɃN���b�v�����`��I�u�W�F�N�g�̃��X�g

    L2DCubismClippingManager* Owner; ///< ���̃}�X�N���Ǘ����Ă���}�l�[�W���̃C���X�^���X
};

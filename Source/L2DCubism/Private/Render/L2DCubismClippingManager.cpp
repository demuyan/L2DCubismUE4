// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubismClippingManager.h"
#include "L2DCubismRenderNormal.h"

///< �t�@�C���X�R�[�v�̕ϐ��錾
namespace
{
    const csmInt32 ColorChannelCount = 4; ///< ��������1�`�����l���̏ꍇ��1�ARGB�����̏ꍇ��3�A�A���t�@���܂߂�ꍇ��4
}

L2DCubismClippingManager::L2DCubismClippingManager()
    :
    CurrentFrameNo(0)
    , ClippingMaskBufferSize(256)
{
    ChannelColors.Add(CSM_NEW FLinearColor(1.0f, 0.0f, 0.0f, 0.0f));
    ChannelColors.Add(CSM_NEW FLinearColor(0.0f, 1.0f, 0.0f, 0.0f));
    ChannelColors.Add(CSM_NEW FLinearColor(0.0f, 0.0f, 1.0f, 0.0f));
    ChannelColors.Add(CSM_NEW FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));
}

L2DCubismClippingManager::~L2DCubismClippingManager()
{
    for (size_t i = 0; i < ClippingContextListForMask.GetSize(); i++)
    {
        if (ClippingContextListForMask[i])
            CSM_DELETE_SELF(L2DCubismClippingContext, ClippingContextListForMask[i]);
        ClippingContextListForMask[i] = nullptr;
    }

    // _clippingContextListForDraw��_clippingContextListForMask�ɂ���C���X�^���X���w���Ă���B��L�̏����ɂ��v�f���Ƃ�DELETE�͕s�v�B
    for (size_t i = 0; i < ClippingContextListForDraw.GetSize(); i++)
    {
        ClippingContextListForDraw[i] = nullptr;
    }

    for (long i = 0; i < ChannelColors.Num(); i++)
    {
        if (ChannelColors[i])
            CSM_DELETE(ChannelColors[i]);
        ChannelColors[i] = nullptr;
    }
}


void L2DCubismClippingManager::Initialize(CubismModel& model)
{
    csmInt32 drawableCount = model.GetDrawableCount();
    const csmInt32** drawableMasks = model.GetDrawableMasks();
    const csmInt32* drawableMaskCounts = model.GetDrawableMaskCounts();

    //�N���b�s���O�}�X�N���g���`��I�u�W�F�N�g��S�ēo�^����
    //�N���b�s���O�}�X�N�́A�ʏ퐔���x�Ɍ��肵�Ďg�����̂Ƃ���
    for (csmInt32 i = 0; i < drawableCount; i++)
    {
        if (drawableMaskCounts[i] <= 0)
        {
            //�N���b�s���O�}�X�N���g�p����Ă��Ȃ��A�[�g���b�V���i�����̏ꍇ�g�p���Ȃ��j
            ClippingContextListForDraw.PushBack(nullptr);
            continue;
        }

        // ���ɂ���ClipContext�Ɠ������`�F�b�N����
        L2DCubismClippingContext* cc = FindSameClip(drawableMasks[i], drawableMaskCounts[i]);
        if (cc == nullptr)
        {
            // ����̃}�X�N�����݂��Ă��Ȃ��ꍇ�͐�������
            cc = CSM_NEW L2DCubismClippingContext(this, drawableMasks[i], drawableMaskCounts[i]);
            ClippingContextListForMask.PushBack(cc);
        }

        cc->AddClippedDrawable(i);
        ClippingContextListForDraw.PushBack(cc);
    }
}

L2DCubismClippingContext* L2DCubismClippingManager::FindSameClip(const csmInt32* drawableMasks,
                                                                 csmInt32 drawableMaskCounts) const
{
    // �쐬�ς�ClippingContext�ƈ�v���邩�m�F
    for (size_t i = 0; i < ClippingContextListForMask.GetSize(); i++)
    {
        L2DCubismClippingContext* cc = ClippingContextListForMask[i];
        const int Count = cc->ClippingIdCount;
        if (Count != drawableMaskCounts) continue; //�����Ⴄ�ꍇ�͕ʕ�
        int Samecount = 0;

        // ����ID�������m�F�B�z��̐��������Ȃ̂ŁA��v�������������Ȃ瓯���������Ƃ���B
        for (int j = 0; j < Count; j++)
        {
            const csmInt32 clipId = cc->ClippingIdList[j];
            for (int k = 0; k < Count; k++)
            {
                if (drawableMasks[k] == clipId)
                {
                    Samecount++;
                    break;
                }
            }
        }
        if (Samecount == Count)
        {
            return cc;
        }
    }
    return nullptr; //������Ȃ�����
}

void L2DCubismClippingManager::SetupClippingContext(FRHICommandListImmediate& RHICmdList, CubismModel& model,
                                                    L2DCubismRenderNormal* renderer)
{
    CurrentFrameNo++;

    // �S�ẴN���b�s���O��p�ӂ���
    // �����N���b�v�i�����̏ꍇ�͂܂Ƃ߂ĂP�̃N���b�v�j���g���ꍇ�͂P�x�����ݒ肷��
    int UsingClipCount = 0;
    for (size_t clipIndex = 0; clipIndex < ClippingContextListForMask.GetSize(); clipIndex++)
    {
        // �P�̃N���b�s���O�}�X�N�Ɋւ���
        L2DCubismClippingContext* cc = ClippingContextListForMask[clipIndex];

        // ���̃N���b�v�𗘗p����`��I�u�W�F�N�g�Q�S�̂��͂ދ�`���v�Z
        CalcClippedDrawTotalBounds(model, cc);

        if (cc->bUsing)
        {
            UsingClipCount++; //�g�p���Ƃ��ăJ�E���g
        }
    }

    // �}�X�N�쐬����
    if (UsingClipCount > 0)
    {
        // �e�}�X�N�̃��C�A�E�g�����肵�Ă���
        SetupLayoutBounds(renderer->IsUsingHighPrecisionMask() ? 0 : UsingClipCount);

        // ���ۂɃ}�X�N�𐶐�����
        // �S�Ẵ}�X�N���ǂ̗l�Ƀ��C�A�E�g���ĕ`���������肵�AClipContext , ClippedDrawContext �ɋL������
        for (size_t ClipIndex = 0; ClipIndex < ClippingContextListForMask.GetSize(); ClipIndex++)
        {
            // --- ���ۂɂP�̃}�X�N��`�� ---
            L2DCubismClippingContext* ClipContext = ClippingContextListForMask[ClipIndex];
            csmRectF* AllClippedDrawRect = ClipContext->AllClippedDrawRect; //���̃}�X�N���g���A�S�Ă̕`��I�u�W�F�N�g�̘_�����W��̈͂݋�`
            csmRectF* LayoutBoundsOnTex01 = ClipContext->LayoutBounds; //���̒��Ƀ}�X�N�����߂�

            // ���f�����W��̋�`���A�K�X�}�[�W����t���Ďg��
            const float_t MARGIN = 0.05f;
            csmRectF TmpBoundsOnModel;
            TmpBoundsOnModel.SetRect(AllClippedDrawRect);
            TmpBoundsOnModel.Expand(AllClippedDrawRect->Width * MARGIN, AllClippedDrawRect->Height * MARGIN);
            //########## �{���͊��蓖�Ă�ꂽ�̈�̑S�̂��g�킸�K�v�Œ���̃T�C�Y���悢

            // �V�F�[�_�p�̌v�Z�������߂�B��]���l�����Ȃ��ꍇ�͈ȉ��̂Ƃ���
            // movePeriod' = movePeriod * scaleX + offX [[ movePeriod' = (movePeriod - tmpBoundsOnModel.movePeriod)*scale + layoutBoundsOnTex01.movePeriod ]]
            const float_t ScaleX = LayoutBoundsOnTex01->Width / TmpBoundsOnModel.Width;
            const float_t ScaleY = LayoutBoundsOnTex01->Height / TmpBoundsOnModel.Height;

            // �}�X�N�������Ɏg���s������߂�
            {
                CubismMatrix44 TmpMatrix; ///< �}�X�N�v�Z�p�̍s��

                // �V�F�[�_�ɓn���s������߂� <<<<<<<<<<<<<<<<<<<<<<<< �v�œK���i�t���Ɍv�Z����΃V���v���ɂł���j
                TmpMatrix.LoadIdentity();
                {
                    // Layout0..1 �� -1..1�ɕϊ�
                    TmpMatrix.TranslateRelative(-1.0f, -1.0f);
                    TmpMatrix.ScaleRelative(2.0f, 2.0f);
                }
                {
                    // view to Layout0..1
                    TmpMatrix.TranslateRelative(LayoutBoundsOnTex01->X, LayoutBoundsOnTex01->Y); //new = [translate]
                    TmpMatrix.ScaleRelative(ScaleX, ScaleY); //new = [translate][scale]
                    TmpMatrix.TranslateRelative(-TmpBoundsOnModel.X, -TmpBoundsOnModel.Y);
                    //new = [translate][scale][translate]
                }
                // tmpMatrixForMask ���v�Z����
                ClipContext->MatrixForMask.SetMatrix(TmpMatrix.GetArray());
            }

            //--------- draw���� mask �Q�Ɨp�s����v�Z
            {
                CubismMatrix44 TmpMatrix; ///< �}�X�N�v�Z�p�̍s��

                // �V�F�[�_�ɓn���s������߂� <<<<<<<<<<<<<<<<<<<<<<<< �v�œK���i�t���Ɍv�Z����΃V���v���ɂł���j
                TmpMatrix.LoadIdentity();
                {
                    TmpMatrix.TranslateRelative(LayoutBoundsOnTex01->X, LayoutBoundsOnTex01->Y); //new = [translate]
                    // �㉺���]
                    TmpMatrix.ScaleRelative(ScaleX, ScaleY * -1.0f); //new = [translate][scale]
                    TmpMatrix.TranslateRelative(-TmpBoundsOnModel.X, -TmpBoundsOnModel.Y);
                    //new = [translate][scale][translate]
                }

                ClipContext->MatrixForDraw.SetMatrix(TmpMatrix.GetArray());
            }

            if (!renderer->IsUsingHighPrecisionMask())
            {
                const csmInt32 ClipDrawCount = ClipContext->ClippingIdCount;
                for (csmInt32 i = 0; i < ClipDrawCount; i++)
                {
                    const csmInt32 ClipDrawIndex = ClipContext->ClippingIdList[i];

                    // ���_��񂪍X�V����Ă��炸�A�M�������Ȃ��ꍇ�͕`����p�X����
                    if (!model.GetDrawableDynamicFlagVertexPositionsDidChange(ClipDrawIndex))
                    {
                        continue;
                    }

                    renderer->IsCulling(model.GetDrawableCulling(ClipDrawIndex) != 0);

                    // �����p�̕ϊ���K�p���ĕ`��
                    // �`�����l�����؂�ւ���K�v������(A,R,G,B)
                    renderer->SetClippingContextBufferForMask(ClipContext);
                    renderer->DrawMeshRHI(
                        RHICmdList,
                        nullptr,
                        model.GetDrawableTextureIndices(ClipDrawIndex),
                        ClipDrawIndex,
                        model.GetDrawableVertexIndexCount(ClipDrawIndex),
                        model.GetDrawableVertexCount(ClipDrawIndex),
                        const_cast<csmUint16*>(model.GetDrawableVertexIndices(ClipDrawIndex)),
                        const_cast<float_t*>(model.GetDrawableVertices(ClipDrawIndex)),
                        reinterpret_cast<float_t*>(const_cast<Live2D::Cubism::Core::csmVector2*>(model.
                            GetDrawableVertexUvs(ClipDrawIndex))),
                        model.GetDrawableOpacity(ClipDrawIndex),
                        CubismRenderer::CubismBlendMode::CubismBlendMode_Normal, //�N���b�s���O�͒ʏ�`�������
                        false // �}�X�N�������̓N���b�s���O�̔��]�g�p�͑S���֌W���Ȃ�
                    );
                }
            }
            else
            {
                // NOP ���̃��[�h�̍ۂ̓`�����l���𕪂����A�}�g���N�X�̌v�Z���������Ă����ĕ`�掩�͖̂{�̕`�撼�O�ōs��
            }
        }

        if (!renderer->IsUsingHighPrecisionMask())
        {
            // useTarget.EndDraw(renderContext);

            renderer->SetClippingContextBufferForMask(nullptr);
        }
    }
}

void L2DCubismClippingManager::CalcClippedDrawTotalBounds(CubismModel& model, L2DCubismClippingContext* ClippingContext)
{
    // ��N���b�s���O�}�X�N�i�}�X�N�����`��I�u�W�F�N�g�j�̑S�̂̋�`
    float_t ClippedDrawTotalMinX = FLT_MAX, ClippedDrawTotalMinY = FLT_MAX;
    float_t ClippedDrawTotalMaxX = FLT_MIN, ClippedDrawTotalMaxY = FLT_MIN;

    // ���̃}�X�N�����ۂɕK�v�����肷��
    // ���̃N���b�s���O�𗘗p����u�`��I�u�W�F�N�g�v���ЂƂł��g�p�\�ł���΃}�X�N�𐶐�����K�v������

    const csmInt32 ClippedDrawCount = ClippingContext->ClippedDrawableIndexList->GetSize();
    for (csmInt32 ClippedDrawableIndex = 0; ClippedDrawableIndex < ClippedDrawCount; ClippedDrawableIndex++)
    {
        // �}�X�N���g�p����`��I�u�W�F�N�g�̕`�悳����`�����߂�
        const csmInt32 DrawableIndex = (*ClippingContext->ClippedDrawableIndexList)[ClippedDrawableIndex];

        const csmInt32 DrawableVertexCount = model.GetDrawableVertexCount(DrawableIndex);
        const float_t* DrawableVertexes = const_cast<float_t*>(model.GetDrawableVertices(DrawableIndex));

        float_t MinX = FLT_MAX, MinY = FLT_MAX;
        float_t MaxX = FLT_MIN, MaxY = FLT_MIN;

        csmInt32 Loop = DrawableVertexCount * Constant::VertexStep;
        for (csmInt32 pi = Constant::VertexOffset; pi < Loop; pi += Constant::VertexStep)
        {
            MinX = FMath::Min(MinX, static_cast<float_t>(DrawableVertexes[pi]));
            MaxX = FMath::Max(MaxX, static_cast<float_t>(DrawableVertexes[pi]));
            MinY = FMath::Min(MinY, static_cast<float_t>(DrawableVertexes[pi + 1]));
            MaxY = FMath::Max(MaxY, static_cast<float_t>(DrawableVertexes[pi + 1]));
        }

        //
        if (MinX == FLT_MAX) continue; //�L���ȓ_���ЂƂ����Ȃ������̂ŃX�L�b�v����

        // �S�̂̋�`�ɔ��f
        ClippedDrawTotalMinX = FMath::Min(ClippedDrawTotalMinX, MinX);
        ClippedDrawTotalMaxX = FMath::Max(ClippedDrawTotalMaxX, MaxX);
        ClippedDrawTotalMinY = FMath::Min(ClippedDrawTotalMinY, MinY);
        ClippedDrawTotalMaxY = FMath::Max(ClippedDrawTotalMaxY, MaxY);
    }

    if (ClippedDrawTotalMinX == FLT_MAX)
    {
        ClippingContext->AllClippedDrawRect->X = 0.0f;
        ClippingContext->AllClippedDrawRect->Y = 0.0f;
        ClippingContext->AllClippedDrawRect->Width = 0.0f;
        ClippingContext->AllClippedDrawRect->Height = 0.0f;
        ClippingContext->bUsing = false;
    }
    else
    {
        ClippingContext->bUsing = true;
        const float_t w = ClippedDrawTotalMaxX - ClippedDrawTotalMinX;
        const float_t h = ClippedDrawTotalMaxY - ClippedDrawTotalMinY;
        ClippingContext->AllClippedDrawRect->X = ClippedDrawTotalMinX;
        ClippingContext->AllClippedDrawRect->Y = ClippedDrawTotalMinY;
        ClippingContext->AllClippedDrawRect->Width = w;
        ClippingContext->AllClippedDrawRect->Height = h;
    }
}

void L2DCubismClippingManager::SetupLayoutBounds(csmInt32 UsingClipCount) const
{
    if (UsingClipCount <= 0)
    {
        // ���̏ꍇ�͈�̃}�X�N�^�[�Q�b�g�𖈉�N���A���Ďg�p����
        for (size_t Index = 0; Index < ClippingContextListForMask.GetSize(); Index++)
        {
            L2DCubismClippingContext* cc = ClippingContextListForMask[Index];
            cc->LayoutChannelNo = 0; // �ǂ�����������̂ŌŒ�ŗǂ�
            cc->LayoutBounds->X = 0.0f;
            cc->LayoutBounds->Y = 0.0f;
            cc->LayoutBounds->Width = 1.0f;
            cc->LayoutBounds->Height = 1.0f;
        }
        return;
    }

    // �ЂƂ�RenderTexture���ɗ͂����ς��Ɏg���ă}�X�N�����C�A�E�g����
    // �}�X�N�O���[�v�̐���4�ȉ��Ȃ�RGBA�e�`�����l���ɂP���}�X�N��z�u���A5�ȏ�6�ȉ��Ȃ�RGBA��2,2,1,1�Ɣz�u����

    // RGBA�����ԂɎg���Ă����B
    const csmInt32 Div = UsingClipCount / ColorChannelCount; //�P�`�����l���ɔz�u�����{�̃}�X�N��
    const csmInt32 Mod = UsingClipCount % ColorChannelCount; //�]��A���̔ԍ��̃`�����l���܂łɂP���z������

    // RGBA���ꂼ��̃`�����l����p�ӂ��Ă���(0:R , 1:G , 2:B, 3:A, )
    csmInt32 CurClipIndex = 0; //���Ԃɐݒ肵�Ă���

    for (csmInt32 ChannelNo = 0; ChannelNo < ColorChannelCount; ChannelNo++)
    {
        // ���̃`�����l���Ƀ��C�A�E�g���鐔
        const csmInt32 LayoutCount = Div + (ChannelNo < Mod ? 1 : 0);

        // �������@�����肷��
        if (LayoutCount == 0)
        {
            // �������Ȃ�
        }
        else if (LayoutCount == 1)
        {
            //�S�Ă����̂܂܎g��
            L2DCubismClippingContext* cc = ClippingContextListForMask[CurClipIndex++];
            cc->LayoutChannelNo = ChannelNo;
            cc->LayoutBounds->X = 0.0f;
            cc->LayoutBounds->Y = 0.0f;
            cc->LayoutBounds->Width = 1.0f;
            cc->LayoutBounds->Height = 1.0f;
        }
        else if (LayoutCount == 2)
        {
            for (csmInt32 i = 0; i < LayoutCount; i++)
            {
                const csmInt32 Xpos = i % 2;

                L2DCubismClippingContext* cc = ClippingContextListForMask[CurClipIndex++];
                cc->LayoutChannelNo = ChannelNo;

                cc->LayoutBounds->X = Xpos * 0.5f;
                cc->LayoutBounds->Y = 0.0f;
                cc->LayoutBounds->Width = 0.5f;
                cc->LayoutBounds->Height = 1.0f;
                //UV��2�ɕ������Ďg��
            }
        }
        else if (LayoutCount <= 4)
        {
            //4�������Ďg��
            for (csmInt32 i = 0; i < LayoutCount; i++)
            {
                const csmInt32 Xpos = i % 2;
                const csmInt32 Ypos = i / 2;

                L2DCubismClippingContext* cc = ClippingContextListForMask[CurClipIndex++];
                cc->LayoutChannelNo = ChannelNo;

                cc->LayoutBounds->X = Xpos * 0.5f;
                cc->LayoutBounds->Y = Ypos * 0.5f;
                cc->LayoutBounds->Width = 0.5f;
                cc->LayoutBounds->Height = 0.5f;
            }
        }
        else if (LayoutCount <= 9)
        {
            //9�������Ďg��
            for (csmInt32 i = 0; i < LayoutCount; i++)
            {
                const csmInt32 Xpos = i % 3;
                const csmInt32 Ypos = i / 3;

                L2DCubismClippingContext* cc = ClippingContextListForMask[CurClipIndex++];
                cc->LayoutChannelNo = ChannelNo;

                cc->LayoutBounds->X = Xpos / 3.0f;
                cc->LayoutBounds->Y = Ypos / 3.0f;
                cc->LayoutBounds->Width = 1.0f / 3.0f;
                cc->LayoutBounds->Height = 1.0f / 3.0f;
            }
        }
        else
        {
            CubismLogError("not supported mask count : %d", LayoutCount);

            // �J�����[�h�̏ꍇ�͒�~������
            CSM_ASSERT(0);

            // �����������s����ꍇ�A SetupShaderProgram�ŃI�[�o�[�A�N�Z�X����������̂Ŏd���Ȃ��K���ɓ���Ă���
            // �������`�挋�ʂ͂낭�Ȃ��ƂɂȂ�Ȃ�
            for (csmInt32 i = 0; i < LayoutCount; i++)
            {
                L2DCubismClippingContext* cc = ClippingContextListForMask[CurClipIndex++];
                cc->LayoutChannelNo = 0;
                cc->LayoutBounds->X = 0.0f;
                cc->LayoutBounds->Y = 0.0f;
                cc->LayoutBounds->Width = 1.0f;
                cc->LayoutBounds->Height = 1.0f;
            }
        }
    }
}

FLinearColor* L2DCubismClippingManager::GetChannelFlagAsColor(csmInt32 channelNo)
{
    return ChannelColors[channelNo];
}

csmVector<L2DCubismClippingContext*>* L2DCubismClippingManager::GetClippingContextListForDraw()
{
    return &ClippingContextListForDraw;
}

void L2DCubismClippingManager::SetClippingMaskBufferSize(csmInt32 size)
{
    ClippingMaskBufferSize = size;
}

csmInt32 L2DCubismClippingManager::GetClippingMaskBufferSize() const
{
    return ClippingMaskBufferSize;
}

/*********************************************************************************************************************
*                                      CubismClippingContext
********************************************************************************************************************/
L2DCubismClippingContext::L2DCubismClippingContext(L2DCubismClippingManager* manager,
                                                   const csmInt32* clippingDrawableIndices, csmInt32 clipCount)
{
    bUsing = false;

    Owner = manager;

    // �N���b�v���Ă���i���}�X�N�p�́jDrawable�̃C���f�b�N�X���X�g
    ClippingIdList = clippingDrawableIndices;

    // �}�X�N�̐�
    ClippingIdCount = clipCount;

    LayoutChannelNo = 0;

    AllClippedDrawRect = CSM_NEW csmRectF();
    LayoutBounds = CSM_NEW csmRectF();

    ClippedDrawableIndexList = CSM_NEW csmVector<csmInt32>();
}

L2DCubismClippingContext::~L2DCubismClippingContext()
{
    if (LayoutBounds != nullptr)
    {
        CSM_DELETE(LayoutBounds);
        LayoutBounds = nullptr;
    }

    if (AllClippedDrawRect != nullptr)
    {
        CSM_DELETE(AllClippedDrawRect);
        AllClippedDrawRect = nullptr;
    }

    if (ClippedDrawableIndexList != nullptr)
    {
        CSM_DELETE(ClippedDrawableIndexList);
        ClippedDrawableIndexList = nullptr;
    }
}

void L2DCubismClippingContext::AddClippedDrawable(csmInt32 drawableIndex)
{
    ClippedDrawableIndexList->PushBack(drawableIndex);
}

L2DCubismClippingManager* L2DCubismClippingContext::GetClippingManager()
{
    return Owner;
}

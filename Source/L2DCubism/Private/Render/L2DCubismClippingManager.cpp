// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubismClippingManager.h"
#include "L2DCubismRenderNormal.h"

///< ファイルスコープの変数宣言
namespace
{
    const csmInt32 ColorChannelCount = 4; ///< 実験時に1チャンネルの場合は1、RGBだけの場合は3、アルファも含める場合は4
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

    // _clippingContextListForDrawは_clippingContextListForMaskにあるインスタンスを指している。上記の処理により要素ごとのDELETEは不要。
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

    //クリッピングマスクを使う描画オブジェクトを全て登録する
    //クリッピングマスクは、通常数個程度に限定して使うものとする
    for (csmInt32 i = 0; i < drawableCount; i++)
    {
        if (drawableMaskCounts[i] <= 0)
        {
            //クリッピングマスクが使用されていないアートメッシュ（多くの場合使用しない）
            ClippingContextListForDraw.PushBack(nullptr);
            continue;
        }

        // 既にあるClipContextと同じかチェックする
        L2DCubismClippingContext* cc = FindSameClip(drawableMasks[i], drawableMaskCounts[i]);
        if (cc == nullptr)
        {
            // 同一のマスクが存在していない場合は生成する
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
    // 作成済みClippingContextと一致するか確認
    for (size_t i = 0; i < ClippingContextListForMask.GetSize(); i++)
    {
        L2DCubismClippingContext* cc = ClippingContextListForMask[i];
        const int Count = cc->ClippingIdCount;
        if (Count != drawableMaskCounts) continue; //個数が違う場合は別物
        int Samecount = 0;

        // 同じIDを持つか確認。配列の数が同じなので、一致した個数が同じなら同じ物を持つとする。
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
    return nullptr; //見つからなかった
}

void L2DCubismClippingManager::SetupClippingContext(FRHICommandListImmediate& RHICmdList, CubismModel& model,
                                                    L2DCubismRenderNormal* renderer)
{
    CurrentFrameNo++;

    // 全てのクリッピングを用意する
    // 同じクリップ（複数の場合はまとめて１つのクリップ）を使う場合は１度だけ設定する
    int UsingClipCount = 0;
    for (size_t clipIndex = 0; clipIndex < ClippingContextListForMask.GetSize(); clipIndex++)
    {
        // １つのクリッピングマスクに関して
        L2DCubismClippingContext* cc = ClippingContextListForMask[clipIndex];

        // このクリップを利用する描画オブジェクト群全体を囲む矩形を計算
        CalcClippedDrawTotalBounds(model, cc);

        if (cc->bUsing)
        {
            UsingClipCount++; //使用中としてカウント
        }
    }

    // マスク作成処理
    if (UsingClipCount > 0)
    {
        // 各マスクのレイアウトを決定していく
        SetupLayoutBounds(renderer->IsUsingHighPrecisionMask() ? 0 : UsingClipCount);

        // 実際にマスクを生成する
        // 全てのマスクをどの様にレイアウトして描くかを決定し、ClipContext , ClippedDrawContext に記憶する
        for (size_t ClipIndex = 0; ClipIndex < ClippingContextListForMask.GetSize(); ClipIndex++)
        {
            // --- 実際に１つのマスクを描く ---
            L2DCubismClippingContext* ClipContext = ClippingContextListForMask[ClipIndex];
            csmRectF* AllClippedDrawRect = ClipContext->AllClippedDrawRect; //このマスクを使う、全ての描画オブジェクトの論理座標上の囲み矩形
            csmRectF* LayoutBoundsOnTex01 = ClipContext->LayoutBounds; //この中にマスクを収める

            // モデル座標上の矩形を、適宜マージンを付けて使う
            const float_t MARGIN = 0.05f;
            csmRectF TmpBoundsOnModel;
            TmpBoundsOnModel.SetRect(AllClippedDrawRect);
            TmpBoundsOnModel.Expand(AllClippedDrawRect->Width * MARGIN, AllClippedDrawRect->Height * MARGIN);
            //########## 本来は割り当てられた領域の全体を使わず必要最低限のサイズがよい

            // シェーダ用の計算式を求める。回転を考慮しない場合は以下のとおり
            // movePeriod' = movePeriod * scaleX + offX [[ movePeriod' = (movePeriod - tmpBoundsOnModel.movePeriod)*scale + layoutBoundsOnTex01.movePeriod ]]
            const float_t ScaleX = LayoutBoundsOnTex01->Width / TmpBoundsOnModel.Width;
            const float_t ScaleY = LayoutBoundsOnTex01->Height / TmpBoundsOnModel.Height;

            // マスク生成時に使う行列を求める
            {
                CubismMatrix44 TmpMatrix; ///< マスク計算用の行列

                // シェーダに渡す行列を求める <<<<<<<<<<<<<<<<<<<<<<<< 要最適化（逆順に計算すればシンプルにできる）
                TmpMatrix.LoadIdentity();
                {
                    // Layout0..1 を -1..1に変換
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
                // tmpMatrixForMask が計算結果
                ClipContext->MatrixForMask.SetMatrix(TmpMatrix.GetArray());
            }

            //--------- draw時の mask 参照用行列を計算
            {
                CubismMatrix44 TmpMatrix; ///< マスク計算用の行列

                // シェーダに渡す行列を求める <<<<<<<<<<<<<<<<<<<<<<<< 要最適化（逆順に計算すればシンプルにできる）
                TmpMatrix.LoadIdentity();
                {
                    TmpMatrix.TranslateRelative(LayoutBoundsOnTex01->X, LayoutBoundsOnTex01->Y); //new = [translate]
                    // 上下反転
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

                    // 頂点情報が更新されておらず、信頼性がない場合は描画をパスする
                    if (!model.GetDrawableDynamicFlagVertexPositionsDidChange(ClipDrawIndex))
                    {
                        continue;
                    }

                    renderer->IsCulling(model.GetDrawableCulling(ClipDrawIndex) != 0);

                    // 今回専用の変換を適用して描く
                    // チャンネルも切り替える必要がある(A,R,G,B)
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
                        CubismRenderer::CubismBlendMode::CubismBlendMode_Normal, //クリッピングは通常描画を強制
                        false // マスク生成時はクリッピングの反転使用は全く関係がない
                    );
                }
            }
            else
            {
                // NOP このモードの際はチャンネルを分けず、マトリクスの計算だけをしておいて描画自体は本体描画直前で行う
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
    // 被クリッピングマスク（マスクされる描画オブジェクト）の全体の矩形
    float_t ClippedDrawTotalMinX = FLT_MAX, ClippedDrawTotalMinY = FLT_MAX;
    float_t ClippedDrawTotalMaxX = FLT_MIN, ClippedDrawTotalMaxY = FLT_MIN;

    // このマスクが実際に必要か判定する
    // このクリッピングを利用する「描画オブジェクト」がひとつでも使用可能であればマスクを生成する必要がある

    const csmInt32 ClippedDrawCount = ClippingContext->ClippedDrawableIndexList->GetSize();
    for (csmInt32 ClippedDrawableIndex = 0; ClippedDrawableIndex < ClippedDrawCount; ClippedDrawableIndex++)
    {
        // マスクを使用する描画オブジェクトの描画される矩形を求める
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
        if (MinX == FLT_MAX) continue; //有効な点がひとつも取れなかったのでスキップする

        // 全体の矩形に反映
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
        // この場合は一つのマスクターゲットを毎回クリアして使用する
        for (size_t Index = 0; Index < ClippingContextListForMask.GetSize(); Index++)
        {
            L2DCubismClippingContext* cc = ClippingContextListForMask[Index];
            cc->LayoutChannelNo = 0; // どうせ毎回消すので固定で良い
            cc->LayoutBounds->X = 0.0f;
            cc->LayoutBounds->Y = 0.0f;
            cc->LayoutBounds->Width = 1.0f;
            cc->LayoutBounds->Height = 1.0f;
        }
        return;
    }

    // ひとつのRenderTextureを極力いっぱいに使ってマスクをレイアウトする
    // マスクグループの数が4以下ならRGBA各チャンネルに１つずつマスクを配置し、5以上6以下ならRGBAを2,2,1,1と配置する

    // RGBAを順番に使っていく。
    const csmInt32 Div = UsingClipCount / ColorChannelCount; //１チャンネルに配置する基本のマスク個数
    const csmInt32 Mod = UsingClipCount % ColorChannelCount; //余り、この番号のチャンネルまでに１つずつ配分する

    // RGBAそれぞれのチャンネルを用意していく(0:R , 1:G , 2:B, 3:A, )
    csmInt32 CurClipIndex = 0; //順番に設定していく

    for (csmInt32 ChannelNo = 0; ChannelNo < ColorChannelCount; ChannelNo++)
    {
        // このチャンネルにレイアウトする数
        const csmInt32 LayoutCount = Div + (ChannelNo < Mod ? 1 : 0);

        // 分割方法を決定する
        if (LayoutCount == 0)
        {
            // 何もしない
        }
        else if (LayoutCount == 1)
        {
            //全てをそのまま使う
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
                //UVを2つに分解して使う
            }
        }
        else if (LayoutCount <= 4)
        {
            //4分割して使う
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
            //9分割して使う
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

            // 開発モードの場合は停止させる
            CSM_ASSERT(0);

            // 引き続き実行する場合、 SetupShaderProgramでオーバーアクセスが発生するので仕方なく適当に入れておく
            // もちろん描画結果はろくなことにならない
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

    // クリップしている（＝マスク用の）Drawableのインデックスリスト
    ClippingIdList = clippingDrawableIndices;

    // マスクの数
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

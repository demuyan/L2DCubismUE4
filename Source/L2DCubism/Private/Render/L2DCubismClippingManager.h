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
     * @brief カラーチャンネル(RGBA)のフラグを取得する
     *
     * @param[in]   channelNo   ->   カラーチャンネル(RGBA)の番号(0:R , 1:G , 2:B, 3:A)
     */
    FLinearColor* GetChannelFlagAsColor(csmInt32 channelNo);

    /**
     * @brief   マスクされる描画オブジェクト群全体を囲む矩形(モデル座標系)を計算する
     *
     * @param[in]   model            ->  モデルのインスタンス
     * @param[in]   ClippingContext  ->  クリッピングマスクのコンテキスト
     */
    void CalcClippedDrawTotalBounds(CubismModel& model, L2DCubismClippingContext* ClippingContext);

    L2DCubismClippingManager();
    virtual ~L2DCubismClippingManager();
    void Initialize(CubismModel& model);

    /**
     * @brief   クリッピングコンテキストを作成する。モデル描画時に実行する。
     *
     * @param[in]   model       ->  モデルのインスタンス
     * @param[in]   renderer    ->  レンダラのインスタンス
     */
    void SetupClippingContext(FRHICommandListImmediate& RHICmdList, CubismModel& model,
                              L2DCubismRenderNormal* renderer);

    /**
     * @brief   既にマスクを作っているかを確認。<br>
     *          作っているようであれば該当するクリッピングマスクのインスタンスを返す。<br>
     *          作っていなければNULLを返す
     *
     * @param[in]   drawableMasks    ->  描画オブジェクトをマスクする描画オブジェクトのリスト
     * @param[in]   drawableMaskCounts ->  描画オブジェクトをマスクする描画オブジェクトの数
     * @return          該当するクリッピングマスクが存在すればインスタンスを返し、なければNULLを返す。
     */
    L2DCubismClippingContext* FindSameClip(const csmInt32* drawableMasks, csmInt32 drawableMaskCounts) const;

    /**
     * @brief   クリッピングコンテキストを配置するレイアウト。<br>
     *           ひとつのレンダーテクスチャを極力いっぱいに使ってマスクをレイアウトする。<br>
     *           マスクグループの数が4以下ならRGBA各チャンネルに１つずつマスクを配置し、5以上6以下ならRGBAを2,2,1,1と配置する。
     *
     * @param[in]   UsingClipCount  ->  配置するクリッピングコンテキストの数
     */
    void SetupLayoutBounds(csmInt32 UsingClipCount) const;

    /**
     * @breif   カラーバッファのアドレスを取得する
     *
     * @return  カラーバッファのアドレス
     */
    // CubismOffscreenFrame_D3D11* GetColorBuffer() const;

    /**
     * @brief   画面描画に使用するクリッピングマスクのリストを取得する
     *
     * @return  画面描画に使用するクリッピングマスクのリスト
     */
    csmVector<L2DCubismClippingContext*>* GetClippingContextListForDraw();

    /**
     *@brief  クリッピングマスクバッファのサイズを設定する
     *
     *@param  size -> クリッピングマスクバッファのサイズ
     *
     */
    void SetClippingMaskBufferSize(csmInt32 size);

    /**
     *@brief  クリッピングマスクバッファのサイズを取得する
     *
     *@return クリッピングマスクバッファのサイズ
     *
     */
    csmInt32 GetClippingMaskBufferSize() const;

    csmInt32 CurrentFrameNo; ///< マスクテクスチャに与えるフレーム番号

    TArray<FLinearColor*> ChannelColors;
    //  csmVector<Live2D::Cubism::Framework::Rendering::CubismRenderer::CubismTextureColor*>  ChannelColors;
    csmVector<L2DCubismClippingContext*> ClippingContextListForMask; ///< マスク用クリッピングコンテキストのリスト
    csmVector<L2DCubismClippingContext*> ClippingContextListForDraw; ///< 描画用クリッピングコンテキストのリスト
    csmInt32 ClippingMaskBufferSize; ///< クリッピングマスクのバッファサイズ（初期値:256）
};


/**
 * @brief   クリッピングマスクのコンテキスト
 */
class L2DCubismClippingContext
{
    friend class L2DCubismClippingManager;
    friend class CubismRenderer;

public:
    /**
     * @brief   引数付きコンストラクタ
     *
     */
    L2DCubismClippingContext(L2DCubismClippingManager* manager, const csmInt32* clippingDrawableIndices,
                             csmInt32 clipCount);

    /**
     * @brief   デストラクタ
     */
    virtual ~L2DCubismClippingContext();

    /**
     * @brief   このマスクにクリップされる描画オブジェクトを追加する
     *
     * @param[in]   drawableIndex   ->  クリッピング対象に追加する描画オブジェクトのインデックス
     */
    void AddClippedDrawable(csmInt32 drawableIndex);

    /**
     * @brief   このマスクを管理するマネージャのインスタンスを取得する。
     *
     * @return  クリッピングマネージャのインスタンス
     */
    L2DCubismClippingManager* GetClippingManager();

    csmBool bUsing; ///< 現在の描画状態でマスクの準備が必要ならtrue
    const csmInt32* ClippingIdList; ///< クリッピングマスクのIDリスト
    csmInt32 ClippingIdCount; ///< クリッピングマスクの数
    csmInt32 LayoutChannelNo; ///< RGBAのいずれのチャンネルにこのクリップを配置するか(0:R , 1:G , 2:B , 3:A)
    csmRectF* LayoutBounds; ///< マスク用チャンネルのどの領域にマスクを入れるか(View座標-1..1, UVは0..1に直す)
    csmRectF* AllClippedDrawRect; ///< このクリッピングで、クリッピングされる全ての描画オブジェクトの囲み矩形（毎回更新）
    CubismMatrix44 MatrixForMask; ///< マスクの位置計算結果を保持する行列
    CubismMatrix44 MatrixForDraw; ///< 描画オブジェクトの位置計算結果を保持する行列
    csmVector<csmInt32>* ClippedDrawableIndexList; ///< このマスクにクリップされる描画オブジェクトのリスト

    L2DCubismClippingManager* Owner; ///< このマスクを管理しているマネージャのインスタンス
};

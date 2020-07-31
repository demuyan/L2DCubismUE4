// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#pragma once

#include "CoreMinimal.h"
#include "L2DCubismRender.h"

#include "Model/CubismModel.hpp"

using namespace Live2D::Cubism;
using namespace Live2D::Cubism::Framework;

/**
 * 
 */
class L2DCubismRender 
{
public: 

    /**
     * @brief   レンダラの初期化処理を実行する<br>
     *           引数に渡したモデルからレンダラの初期化処理に必要な情報を取り出すことができる
     *
     * @param[in]  model -> モデルのインスタンス
     */
    virtual void Initialize(Framework::CubismModel* model);

    /**
     * @brief  カリング（片面描画）の有効・無効をセットする。<br>
     *          有効にするならtrue, 無効にするならfalseをセットする。
     */
    void SetCulling(csmBool culling);

    /**
     * @brief  カリング（片面描画）の有効・無効を取得する。
     *
     * @retval  true    ->  カリング有効
     * @retval  false   ->  カリング無効
     */
    csmBool IsCulling() const;

    /**
     * @brief   マスク描画の方式を変更する。
     *           falseの場合、マスクを1枚のテクスチャに分割してレンダリングする（デフォルトはこちら）。
     *           高速だが、マスク個数の上限が36に限定され、質も荒くなる。
     *           trueの場合、パーツ描画の前にその都度必要なマスクを描き直す。
     *           レンダリング品質は高いが描画処理負荷は増す。
     */
    void UseHighPrecisionMask(csmBool high);

    /**
     * @brief   マスク描画の方式を取得する。
     */
    bool IsUsingHighPrecisionMask();

    /*
     * FMatrixへ変換
     */
    static FMatrix ConvertToFMatrix(Csm::CubismMatrix44& InCubismMartix);
    
protected:

    L2DCubismRender();
    virtual ~L2DCubismRender();
    
private:
    L2DCubismRender(const L2DCubismRender&);
    L2DCubismRender& operator=(const L2DCubismRender&);

    bool    bCulling;             ///< カリングが有効ならtrue
    bool    bUseHighPrecisionMask;  ///< falseの場合、マスクを纏めて描画する trueの場合、マスクはパーツ描画ごとに書き直す
};

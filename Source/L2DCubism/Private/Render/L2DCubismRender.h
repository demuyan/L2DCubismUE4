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
     * @brief   �����_���̏��������������s����<br>
     *           �����ɓn�������f�����烌���_���̏����������ɕK�v�ȏ������o�����Ƃ��ł���
     *
     * @param[in]  model -> ���f���̃C���X�^���X
     */
    virtual void Initialize(Framework::CubismModel* model);

    /**
     * @brief  �J�����O�i�Жʕ`��j�̗L���E�������Z�b�g����B<br>
     *          �L���ɂ���Ȃ�true, �����ɂ���Ȃ�false���Z�b�g����B
     */
    void SetCulling(csmBool culling);

    /**
     * @brief  �J�����O�i�Жʕ`��j�̗L���E�������擾����B
     *
     * @retval  true    ->  �J�����O�L��
     * @retval  false   ->  �J�����O����
     */
    csmBool IsCulling() const;

    /**
     * @brief   �}�X�N�`��̕�����ύX����B
     *           false�̏ꍇ�A�}�X�N��1���̃e�N�X�`���ɕ������ă����_�����O����i�f�t�H���g�͂�����j�B
     *           ���������A�}�X�N���̏����36�Ɍ��肳��A�����r���Ȃ�B
     *           true�̏ꍇ�A�p�[�c�`��̑O�ɂ��̓s�x�K�v�ȃ}�X�N��`�������B
     *           �����_�����O�i���͍������`�揈�����ׂ͑����B
     */
    void UseHighPrecisionMask(csmBool high);

    /**
     * @brief   �}�X�N�`��̕������擾����B
     */
    bool IsUsingHighPrecisionMask();

    /*
     * FMatrix�֕ϊ�
     */
    static FMatrix ConvertToFMatrix(Csm::CubismMatrix44& InCubismMartix);
    
protected:

    L2DCubismRender();
    virtual ~L2DCubismRender();
    
private:
    L2DCubismRender(const L2DCubismRender&);
    L2DCubismRender& operator=(const L2DCubismRender&);

    bool    bCulling;             ///< �J�����O���L���Ȃ�true
    bool    bUseHighPrecisionMask;  ///< false�̏ꍇ�A�}�X�N��Z�߂ĕ`�悷�� true�̏ꍇ�A�}�X�N�̓p�[�c�`�悲�Ƃɏ�������
};

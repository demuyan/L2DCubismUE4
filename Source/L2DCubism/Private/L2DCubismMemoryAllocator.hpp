// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#pragma once

#include <CubismFramework.hpp>
#include <ICubismAllocator.hpp>

class L2DCubismMemoryAllocator : public Csm::ICubismAllocator
{
    void* Allocate(Csm::csmSizeType size) override;
    void Deallocate(void* memory) override;
    void* AllocateAligned(Csm::csmSizeType size, Csm::csmUint32 alignment) override;
    void DeallocateAligned(void* alignedMemory) override;
};

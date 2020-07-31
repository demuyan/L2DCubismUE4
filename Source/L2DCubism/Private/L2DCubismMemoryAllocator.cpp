// Copyright 2020 demuyan
// SPDX-License-Identifier: MIT
// Licensed under the MIT Open Source License, for details please see license.txt or the website
// http://www.opensource.org/licenses/mit-license.php

#include "L2DCubismMemoryAllocator.hpp"
#include "HAL/UnrealMemory.h"

using namespace Csm;

void* L2DCubismMemoryAllocator::Allocate(const csmSizeType size)
{
    return static_cast<void*>(FMemory::Malloc(size));
}

void L2DCubismMemoryAllocator::Deallocate(void* memory)
{
    FMemory::Free(memory);
}

void* L2DCubismMemoryAllocator::AllocateAligned(const csmSizeType size, const csmUint32 alignment)
{
    return static_cast<void*>(FMemory::Malloc(size, alignment));
}

void L2DCubismMemoryAllocator::DeallocateAligned(void* alignedMemory)
{
    FMemory::Free(alignedMemory);
}

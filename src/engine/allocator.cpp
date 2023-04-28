//
//  allocator.cpp
//  FrameTech
//
//  Created by Antonin on 31/12/2022.
//

// This header is only present for VMA implementation.
// Do not modify or remove.

#define VMA_IMPLEMENTATION

#ifdef DEBUG
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_DEBUG_DETECT_CORRUPTION 1
#define VMA_RECORDING_ENABLED
#endif

#include <vk_mem_alloc.h> // VMA

//
//   Copyright (c) 2025 Nicholas Marino
//   All rights reserved.
//

#include "GCLContext.h"
#include "VulkanContext.in.h"

#define VMA_IMPLEMENTATION
#include "../vendor/vma.h"

#include <memory>

using namespace gcl;

GCLContext::GCLContext() {
    m_context = std::make_unique<VulkanContext>();
}

GCLContext::~GCLContext() {
    
}

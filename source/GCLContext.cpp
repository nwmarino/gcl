//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#include "../include/GCLContext.h"
#include "VulkanContext.in.h"

#include <memory>

using namespace gcl;

GCLContext::GCLContext() {
    m_context = std::make_unique<VulkanContext>();
}

GCLContext::~GCLContext() {
    
}

GCLContext::operator VkDevice() const {
    return m_context->get_device();
}

GCLContext::operator VmaAllocator() const {
    return m_context->get_allocator();
}

VkDevice GCLContext::get_device() const {
    return m_context->get_device();
}

VmaAllocator GCLContext::get_allocator() const {
    return m_context->get_allocator();
}

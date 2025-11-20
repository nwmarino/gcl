//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#include "../include/Buffer.h"
#include "VulkanContext.in.h"
#include <vulkan/vulkan_core.h>

using namespace gcl;

Buffer::Buffer(GCLContext& context, uint64_t size) 
        : m_context(context), m_size(static_cast<VkDeviceSize>(size)) {
    VkBufferCreateInfo buf_info {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.size = size;
    buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VmaAllocationCreateInfo alloc_info {};
    alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VK_CHECK(vmaCreateBuffer(
        m_context, 
        &buf_info, 
        &alloc_info, 
        &m_buf, 
        &m_alloc, 
        nullptr));
}

Buffer::~Buffer() {
    vmaDestroyBuffer(m_context, m_buf, m_alloc);

    m_buf = nullptr;
    m_alloc = nullptr;
}

void Buffer::map(void** out) const {
    void* data;
    vmaMapMemory(m_context, m_alloc, &data);
    *out = reinterpret_cast<void*>(data);
}

void Buffer::unmap() const {
    vmaUnmapMemory(m_context, m_alloc);
}

void Buffer::flush() const {
    vmaFlushAllocation(m_context, m_alloc, 0, VK_WHOLE_SIZE);
}

void Buffer::invalidate() const {
    vmaInvalidateAllocation(m_context, m_alloc, 0, VK_WHOLE_SIZE);
}

//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#ifndef GCL_BUFFER_H_
#define GCL_BUFFER_H_

#include "GCLContext.h"

#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace gcl {

template<typename T>
class Buffer final {
    GCLContext& m_context;

    /// The underlying Vulkan buffer.
    VkBuffer m_buf;

    /// The size of this buffer in bytes. This is determined by the size of
    /// the template parameter and the # of elements designated in the ctor.
    VkDeviceSize m_size;

    /// The corresponding VMA device memory allocation.
    VmaAllocation m_alloc;

public:
    /// Create a new buffer with the number of elements |N|.
    Buffer(GCLContext& context, uint64_t N) 
            : m_context(context), m_size(sizeof(T) * N) {
        VkBufferCreateInfo buf_info {};
        buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buf_info.size = m_size;

        VmaAllocationCreateInfo alloc_info {};
        alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        VK_CHECK(vmaCreateBuffer(
            m_context, &buf_info, &alloc_info, &m_buf, &m_alloc, nullptr));
    }

    Buffer(const Buffer&) = delete;
    Buffer& operator = (const Buffer&) = delete;

    ~Buffer() {
        vmaDestroyBuffer(m_context, m_buf, m_alloc);

        m_buf = nullptr;
        m_alloc = nullptr;
    }

    operator VkBuffer() const { return m_buf; }

    /// Returns the size of this buffer in bytes.
    uint64_t size() const { return static_cast<uint64_t>(m_size); }

    /// Returns the number of elements which can fit into this buffer based on
    /// the template type.
    uint64_t elements() const { 
        return static_cast<uint64_t>(m_size) / sizeof(T); 
    }

    void send(const std::vector<T>& data) const {
        void* p = nullptr;
        map(&p);

        for (uint32_t i = 0; i < data.size(); ++i)
            static_cast<T*>(p)[i] = data[i];

        flush();
        unmap();
    }

    std::vector<T> fetch() const {
        invalidate();

        void* p = nullptr;
        map(&p);

        std::vector<T> data(elements());
        for (uint32_t i = 0; i < data.size(); ++i)
            data[i] = static_cast<T*>(p)[i];
        
        unmap();
        return data;
    }

    void map(void** out) const {
        void* data;
        vmaMapMemory(m_context, m_alloc, &data);
        *out = reinterpret_cast<void*>(data);
    }

    void unmap() const {
        vmaUnmapMemory(m_context, m_alloc);
    }

    void flush() const {
        vmaFlushAllocation(m_context, m_alloc, 0, VK_WHOLE_SIZE);
    }

    void invalidate() const {
        vmaInvalidateAllocation(m_context, m_alloc, 0, VK_WHOLE_SIZE);
    }
};

} // namespace gcl

#endif // GCL_BUFFER_H_

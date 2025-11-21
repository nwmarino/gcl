//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#ifndef GCL_BUFFER_H_
#define GCL_BUFFER_H_

#include "GCLContext.h"

#include <cstdint>

namespace gcl {

class Buffer final {
    GCLContext& m_context;
    VkBuffer m_buf;
    VkDeviceSize m_size;
    VmaAllocation m_alloc;

public:
    Buffer(GCLContext& context, uint64_t size);

    Buffer(const Buffer&) = delete;
    Buffer& operator = (const Buffer&) = delete;

    ~Buffer();

    operator VkBuffer() const { return m_buf; }

    void map(void** out) const;

    void unmap() const;

    /// Returns the size of this buffer in bytes.
    uint64_t size() const { return static_cast<uint64_t>(m_size); }

    void flush() const;

    void invalidate() const;
};

} // namespace gcl

#endif // GCL_BUFFER_H_

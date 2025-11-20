//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#ifndef GCL_CONTEXT_H_
#define GCL_CONTEXT_H_

#include "../vendor/vma.h"

#include <memory>

namespace gcl {

class VulkanContext;

class GCLContext {
    friend class Buffer;
    friend class Kernel;

    std::unique_ptr<VulkanContext> m_context;

public:
    GCLContext();

    GCLContext(const GCLContext&) = delete;
    GCLContext& operator = (const GCLContext&) = delete;

    ~GCLContext();

    operator VkDevice() const;
    operator VmaAllocator() const;

    VkDevice get_device() const;

    VmaAllocator get_allocator() const;
};

} // namespace gcl

#endif // GCL_CONTEXT_H_

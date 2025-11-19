//
//   Copyright (c) 2025 Nicholas Marino
//   All rights reserved.
//

#ifndef GCL_CONTEXT_H_
#define GCL_CONTEXT_H_

#include "../vendor/vma.h"

#include <memory>

namespace gcl {

class VulkanContext;

class GCLContext {;
    std::unique_ptr<VulkanContext> m_context;

public:
    GCLContext();

    GCLContext(const GCLContext&) = delete;
    GCLContext& operator = (const GCLContext&) = delete;

    ~GCLContext();

    operator VmaAllocator() const;

    VmaAllocator get_allocator() const;
};

} // namespace gcl

#endif // GCL_CONTEXT_H_

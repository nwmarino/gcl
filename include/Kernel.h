//
//   Copyright (c) 2025 Nicholas Marino
//   All rights reserved.
//

#ifndef GCL_KERNEL_H_
#define GCL_KERNEL_H_

#include "Buffer.h"
#include "GCLContext.h"

#include <vulkan/vulkan.h>

#include <string>

namespace gcl {

class Kernel final {
    GCLContext& m_context;
    VkShaderModule m_compute = nullptr;
    VkPipelineLayout m_layout = nullptr;
    VkPipeline m_pipeline = nullptr;

    void init_vulkan_compute_shader(const std::string& compute);

    void init_vulkan_compute_pipeline();

public:
    Kernel(GCLContext& context, const std::string& compute);

    Kernel(const Kernel&) = delete;
    Kernel& operator=(const Kernel&) = delete;

    ~Kernel();

    void dispatch(int32_t xgroups, int32_t ygroups = 1, int32_t zgroups = 1);

    void bind(uint32_t binding, Buffer& buf);
};

} // namespace gcl

#endif // GCL_KERNEL_H_

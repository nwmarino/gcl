//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#ifndef GCL_KERNEL_H_
#define GCL_KERNEL_H_

#include "Buffer.h"
#include "GCLContext.h"

#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace gcl {

class Kernel final {
    GCLContext& m_context;
    VkShaderModule m_compute = nullptr;
    VkPipelineLayout m_layout = nullptr;
    VkPipeline m_pipeline = nullptr;

    VkDescriptorSetLayout m_desc_layout = nullptr;
    VkDescriptorPool m_desc_pool = nullptr;
    VkDescriptorSet m_desc_set = nullptr;

    uint32_t m_local_size_x = 1;

    void init_vulkan_compute_shader(const std::string& compute);

    void init_vulkan_compute_pipeline();

    void reflect_descriptors(const std::vector<char>& spv);

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

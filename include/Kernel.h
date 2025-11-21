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

    void dispatch(int32_t xelements, int32_t ygroups = 1, int32_t zgroups = 1);

    template<typename T>
    void bind(uint32_t binding, Buffer<T>& buf) {
        VkDescriptorBufferInfo info {};
        info.buffer = buf;
        info.range = buf.size();

        VkWriteDescriptorSet write {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write.descriptorCount = 1;
        write.dstBinding = binding;
        write.dstSet = m_desc_set;
        write.pBufferInfo = &info;

        vkUpdateDescriptorSets(m_context, 1, &write, 0, nullptr);
    }
};

} // namespace gcl

#endif // GCL_KERNEL_H_

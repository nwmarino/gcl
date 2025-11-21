//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#include "../include/Kernel.h"

#include "../vendor/spirv_reflect.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <unordered_map>
#include <vector>

using namespace gcl;

static std::vector<char> read_file(const std::string& path) {
    // Open the file in binary mode and seek to the end to get the size.
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (file.is_open() == false)
        throw rt_error("failed to open file: " + path);

    uint64_t size = file.tellg();
    std::vector<char> buf(size);
    file.seekg(0);

    // Read the file into the buffer.
    if (!file.read(buf.data(), size))
        throw rt_error("failed to read file: " + path);

    file.close();
    return buf;
}

Kernel::Kernel(GCLContext& context, const std::string& compute) 
        : m_context(context) {
    init_vulkan_compute_shader(compute);
    init_vulkan_compute_pipeline();
}

Kernel::~Kernel() {
    if (m_desc_pool != nullptr) {
        vkDestroyDescriptorPool(m_context, m_desc_pool, nullptr);
        m_desc_pool = nullptr;
    }

    if (m_desc_layout != nullptr) {
        vkDestroyDescriptorSetLayout(m_context, m_desc_layout, nullptr);
        m_desc_layout = nullptr;
    }

    if (m_pipeline != nullptr) {
        vkDestroyPipeline(m_context, m_pipeline, nullptr);
        m_pipeline = nullptr;
    }

    if (m_layout != nullptr) {
        vkDestroyPipelineLayout(m_context, m_layout, nullptr);
        m_layout = nullptr;
    }
    
    if (m_compute != nullptr) {
        vkDestroyShaderModule(m_context, m_compute, nullptr);
        m_compute = nullptr;
    }
}

void Kernel::init_vulkan_compute_shader(const std::string& compute) {
    std::vector<char> spv = read_file(compute);

    reflect_descriptors(spv);

    VkShaderModuleCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = spv.size();
    info.pCode = reinterpret_cast<const uint32_t*>(spv.data());

    VK_CHECK(vkCreateShaderModule(m_context, &info, nullptr, &m_compute));
}

void Kernel::init_vulkan_compute_pipeline() {
    VkPipelineLayoutCreateInfo layout_info {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (m_desc_layout != nullptr) {
        layout_info.setLayoutCount = 1;
        layout_info.pSetLayouts = &m_desc_layout;
    }

    VK_CHECK(vkCreatePipelineLayout(
        m_context, &layout_info, nullptr, &m_layout));

    VkPipelineShaderStageCreateInfo stage_info {};
    stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage_info.module = m_compute;
    stage_info.pName = "main";

    VkComputePipelineCreateInfo pipeline_info {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.stage = stage_info;
    pipeline_info.layout = m_layout;

    VK_CHECK(vkCreateComputePipelines(
        m_context, nullptr, 1, &pipeline_info, nullptr, &m_pipeline));
}

void Kernel::reflect_descriptors(const std::vector<char>& spv) {
    SpvReflectShaderModule module {};
    SpvReflectResult res = spvReflectCreateShaderModule(spv.size(), spv.data(), &module);
    if (res != SPV_REFLECT_RESULT_SUCCESS)
        throw rt_error("(SPIRV-Reflect) failed to make shader module for reflection.");

    m_local_size_x = std::max(
        static_cast<uint32_t>(1), module.entry_points[0].local_size.x);

    uint32_t num_sets = 0;
    res = spvReflectEnumerateDescriptorSets(&module, &num_sets, nullptr);
    if (res != SPV_REFLECT_RESULT_SUCCESS) {
        spvReflectDestroyShaderModule(&module);
        throw rt_error("(SPIRV-Reflect) failed to list descriptor sets.");
    }

    std::vector<SpvReflectDescriptorSet*> sets(num_sets);
    res = spvReflectEnumerateDescriptorSets(&module, &num_sets, sets.data());
    if (res != SPV_REFLECT_RESULT_SUCCESS) {
        spvReflectDestroyShaderModule(&module);
        throw rt_error("(SPIRV-Reflect) failed to reflect descriptor sets.");
    }

    // TODO: Support more than one descriptor set.
    SpvReflectDescriptorSet* set0 = nullptr;
    for (auto* s : sets) {
        if (s->set == 0) {
            set0 = s;
            break;
        }
    }

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<VkDescriptorPoolSize> pool_sizes;

    if (set0 != nullptr) {
        bindings.reserve(set0->binding_count);
        std::unordered_map<VkDescriptorType, uint32_t> type_counts = {};

        for (uint32_t idx = 0; idx < set0->binding_count; ++idx) {
            const SpvReflectDescriptorBinding* rb = set0->bindings[idx];

            VkDescriptorSetLayoutBinding binding {};
            binding.binding = rb->binding;
            binding.descriptorType = 
                static_cast<VkDescriptorType>(rb->descriptor_type);
            binding.descriptorCount = rb->count;
            binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

            bindings.push_back(binding);

            type_counts[binding.descriptorType] += binding.descriptorCount;
        }

        pool_sizes.reserve(type_counts.size());
        for (const auto& [type, count] : type_counts) {
            VkDescriptorPoolSize size {};
            size.type = type;
            size.descriptorCount = count;
            pool_sizes.push_back(size);
        }

        VkDescriptorSetLayoutCreateInfo layout_info {};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
        layout_info.pBindings = bindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(
            m_context, &layout_info, nullptr, &m_desc_layout));

        VkDescriptorPoolCreateInfo pool_info {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.maxSets = 1;
        pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        pool_info.pPoolSizes = pool_sizes.data();

        VK_CHECK(vkCreateDescriptorPool(
            m_context, &pool_info, nullptr, &m_desc_pool));

        VkDescriptorSetAllocateInfo alloc_info {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = m_desc_pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &m_desc_layout;

        VK_CHECK(vkAllocateDescriptorSets(
            m_context, &alloc_info, &m_desc_set));
    }

    spvReflectDestroyShaderModule(&module);
}

void Kernel::dispatch(int32_t xelements, int32_t ygroups, int32_t zgroups) {
    if (xelements == 0 || ygroups == 0 || zgroups == 0)
        return;

    uint32_t groups_x = (xelements + m_local_size_x - 1u) / m_local_size_x;

    VkCommandBuffer cmd = m_context.get_command_buffer();
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_CHECK(vkBeginCommandBuffer(cmd, &begin_info));

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
    if (m_desc_set != nullptr) {
        vkCmdBindDescriptorSets(
            cmd, 
            VK_PIPELINE_BIND_POINT_COMPUTE, 
            m_layout, 
            0, 
            1, 
            &m_desc_set, 
            0, 
            nullptr);
    }

    vkCmdDispatch(cmd, groups_x, ygroups, zgroups);
    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo submit {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    VkFence fence = m_context.get_fence();

    VK_CHECK(vkResetFences(m_context, 1, &fence));
    VK_CHECK(vkQueueSubmit(m_context.get_compute_queue(), 1, &submit, fence));
    VK_CHECK(vkWaitForFences(m_context, 1, &fence, VK_TRUE, UINT64_MAX));
}

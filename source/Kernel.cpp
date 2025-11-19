//
//   Copyright (c) 2025 Nicholas Marino
//   All rights reserved.
//

#include "../include/Kernel.h"
#include "VulkanContext.in.h"

#include <fstream>
#include <vector>
#include <vulkan/vulkan_core.h>

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
    std::vector<char> code = read_file(compute);

    VkShaderModuleCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = code.size();
    info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VK_CHECK(vkCreateShaderModule(m_context, &info, nullptr, &m_compute));
}

void Kernel::init_vulkan_compute_pipeline() {
    VkPipelineLayoutCreateInfo layout_info {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

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

void Kernel::dispatch(int32_t xgroups, int32_t ygroups, int32_t zgroups) {
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkCommandBuffer cmd = m_context.m_context->get_command_buffer();

    if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS)
        throw rt_error("failed to begin recording command buffer.");

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);

    /*
    vkCmdBindDescriptorSets(
        cmd, 
        VK_PIPELINE_BIND_POINT_COMPUTE, 
        m_layout, 
        0, 
        1, 
        nullptr, 
        0, 
        0);
    */

    vkCmdDispatch(cmd, (xgroups / 256) + 1, ygroups, zgroups);

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
        throw rt_error("failed to record command buffer.");
}

void Kernel::bind(uint32_t binding, Buffer& buf) {
    
}

//
//   Copyright (c) 2025 Nicholas Marino
//   All rights reserved.
//

#ifndef GCL_VULKAN_CONTEXT_H_
#define GCL_VULKAN_CONTEXT_H_

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#ifndef NDEBUG
    #define USE_VALIDATION_LAYERS
#endif

namespace gcl {

/// A layer over Vulkan-related objects to provide compute shaders and buffers
/// on the GPU.
class VulkanContext final {
    friend class GCLContext;

    VkInstance m_instance = nullptr;
    VkPhysicalDevice m_physical_device = nullptr;
    VkDevice m_device = nullptr;
    VkQueue m_queue = nullptr;
    uint32_t m_qfamily = 0;

#ifdef USE_VALIDATION_LAYERS
    VkDebugUtilsMessengerEXT m_msger = nullptr;
#endif // USE_VALIDATION_LAYERS

    /// Initialize the Vulkan instance object for this context.
    void init_vulkan_instance();

    /// Initialize the Vulkan physical device object for this context.
    void init_vulkan_physical_device();

    /// Initialize the Vulkan logical device object for this context.
    void init_vulkan_logical_device();

public:
    VulkanContext();

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator = (const VulkanContext&) = delete;

    ~VulkanContext();

    operator VkInstance() const { return m_instance; }
    operator VkDevice() const { return m_device; }
    operator VkPhysicalDevice() const { return m_physical_device; }

    /// Returns the Vulkan instance used in this context.
    VkInstance get_instance() const { return m_instance; }

    /// Returns the Vulkan logical device used in this context.
    VkDevice get_device() const { return m_device; }

    /// Returns the Vulkan physical device used in this context.
    VkPhysicalDevice get_physical_device() const { return m_physical_device; }

    /// Returns the Vulkan compute queue used in this context.
    VkQueue get_compute_queue() const { return m_queue; }

    /// Returns the queue family index for the compute queue used in this 
    /// context.
    uint32_t get_compute_queue_family() const { return m_qfamily; }
};

} // namespace gcl

#endif // GCL_VULKAN_CONTEXT_H_

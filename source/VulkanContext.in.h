#ifndef GCL_VULKAN_CONTEXT_H_
#define GCL_VULKAN_CONTEXT_H_

#include <vulkan/vulkan.h>

namespace gcl {

class VulkanContext final {
    friend class GCLContext;

    VkInstance m_instance = nullptr;
    VkDebugUtilsMessengerEXT m_msger = nullptr;
    VkPhysicalDevice m_physical_device = nullptr;
    VkDevice m_device = nullptr;

    VulkanContext();

    /// Initialize the Vulkan instance object for this context.
    void init_vulkan_instance();

    /// Initialize the Vulkan physical device object for this context.
    void init_vulkan_physical_device();

    /// Initialize the Vulkan logical device object for this context.
    void init_vulkan_logical_device();

public:
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator = (const VulkanContext&) = delete;

    ~VulkanContext();

    operator VkDevice() const { return m_device; }

    operator VkPhysicalDevice() const { return m_physical_device; }

    /// Returns the Vulkan logical device used in this context.
    VkDevice get_device() const { return m_device; }

    /// Returns the Vulkan physical device used in this context.
    VkPhysicalDevice get_physical_device() const { return m_physical_device; }
};

} // namespace gcl

#endif // GCL_VULKAN_CONTEXT_H_

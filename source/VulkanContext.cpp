//
//   Copyright (c) 2025 Nicholas Marino
//   All rights reserved.
//

#include "VulkanContext.in.h"

#include <vulkan/vk_enum_string_helper.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <optional>
#include <vector>

using rt_error = std::runtime_error;

using namespace gcl;

#define VK_CHECK(x)                                                        \
    do {                                                                   \
        VkResult e = x;                                                    \
        if (e) {                                                           \
            throw rt_error("(Vulkan) " + std::string(string_VkResult(e))); \
            abort();                                                       \
        }                                                                  \
    } while (0)

/// Check the available Vulkan layers for basic validation support.
static bool has_validation_layer_support() {
    uint32_t num_layers;
    vkEnumerateInstanceLayerProperties(&num_layers, nullptr);

    std::vector<VkLayerProperties> layers(num_layers);
    vkEnumerateInstanceLayerProperties(&num_layers, layers.data());

    for (const auto& props : layers) {
        if (0 == std::strcmp(props.layerName, "VK_LAYER_KHRONOS_validation"))
            return true;
    }
    
    return false;
}

/// Finds and returns the index of a queue family that supports compute.
static std::optional<uint32_t> find_compute_queue_index(
        VkPhysicalDevice device) {
    uint32_t num_families;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &num_families, nullptr);

    std::vector<VkQueueFamilyProperties> families(num_families);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &num_families, families.data());

    uint32_t iter = 0;
    std::optional<uint32_t> index = std::nullopt;

    for (const auto& family : families) {
        if (family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            index = iter;
            break;
        }

        ++iter;
    }

    return index;
}

#ifdef USE_VALIDATION_LAYERS
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data) {
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        std::cerr << "(Vulkan) " << callback_data->pMessage;
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        std::cerr << "(Vulkan) " << callback_data->pMessage;
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        std::cerr << "(Vulkan) " << callback_data->pMessage;
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        break;
    }

    return VK_FALSE;
}
#endif // USE_VALIDATION_LAYERS

VulkanContext::VulkanContext() {
    init_vulkan_instance();
    init_vulkan_physical_device();
    init_vulkan_logical_device();
}

VulkanContext::~VulkanContext() {
    if (m_device != nullptr)
        vkDeviceWaitIdle(m_device);

    vkDestroyDevice(m_device, nullptr);

#ifdef USE_VALIDATION_LAYERS
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_msger, nullptr);
#endif // USE_VALIDATION_LAYERS

    vkDestroyInstance(m_instance, nullptr);
}

void VulkanContext::init_vulkan_instance() {
    VkApplicationInfo app_info {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "gcl";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "gcl";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

#ifdef USE_VALIDATION_LAYERS
    if (!has_validation_layer_support())
        throw rt_error("validation layers requested, but unavailable.");

    VkDebugUtilsMessengerCreateInfoEXT msger_info {};
    msger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    msger_info.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    msger_info.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT 
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    msger_info.pfnUserCallback = debug_callback;

    const char* validation_layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayerNames = validation_layers;
    create_info.pNext = &msger_info;
#endif // USE_VALIDATION_LAYERS

    VK_CHECK(vkCreateInstance(&create_info, nullptr, &m_instance));

#ifdef USE_VALIDATION_LAYERS
    VK_CHECK(vkCreateDebugUtilsMessengerEXT(
        m_instance, &msger_info, nullptr, &m_msger));
#endif // USE_VALIDATION_LAYERS
}

void VulkanContext::init_vulkan_physical_device() {
    uint32_t num_devices;
    vkEnumeratePhysicalDevices(m_instance, &num_devices, nullptr);

    if (num_devices == 0)
        throw rt_error("no physical devices available.");

    std::vector<VkPhysicalDevice> devices(num_devices);
    vkEnumeratePhysicalDevices(m_instance, &num_devices, devices.data());

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        // implement some restrictions here on whether a device is suitable.
        
        // for now, choose first device available.
        m_physical_device = device;
        break;
    }
}

void VulkanContext::init_vulkan_logical_device() {
    std::optional<uint32_t> compute_index = 
        find_compute_queue_index(m_physical_device);

    if (!compute_index.has_value())
        throw rt_error("physical device not support a compute queue.");

    float queue_prio = 1.f;
    VkDeviceQueueCreateInfo queue_info {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = *compute_index;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &queue_prio;
    
    VkPhysicalDeviceFeatures core {};
    // no core features needed.
    
    VkPhysicalDeviceVulkan12Features v12 {};
    v12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    v12.bufferDeviceAddress = VK_TRUE;
    v12.descriptorIndexing = VK_TRUE;

    VkPhysicalDeviceVulkan13Features v13 {};
    v13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    // no 1.3 features needed.
    
    VkPhysicalDeviceFeatures2 feats {};
    feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    feats.features = core;
    feats.pNext = &v13;

    v13.pNext = &v12;

    VkDeviceCreateInfo device_info {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.pNext = &feats;

    VK_CHECK(vkCreateDevice(
        m_physical_device, &device_info, nullptr, &m_device));

    // Get the compute queue we asked for.
    vkGetDeviceQueue(m_device, *compute_index, 0, &m_queue);
    m_qfamily = *compute_index;
}

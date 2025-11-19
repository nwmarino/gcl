//
//   Copyright (c) 2025 Nicholas Marino
//   All rights reserved.
//

#include "VulkanContext.in.h"
#include <vulkan/vulkan_core.h>

#define VMA_IMPLEMENTATION
#include "../vendor/vma.h"

#include <cstdint>
#include <iostream>
#include <set>
#include <optional>
#include <vector>

using namespace gcl;

std::vector<const char*> EXTENSIONS = {
#ifdef USE_VALIDATION_LAYERS
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif // USE_VALIDATION_LAYERS
};

#ifdef USE_VALIDATION_LAYERS

#include <cstring>

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

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data) {
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        std::cerr << "(Vulkan) " << callback_data->pMessage << '\n';
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        std::cerr << "(Vulkan) " << callback_data->pMessage << '\n'; 
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        std::cerr << "(Vulkan) " << callback_data->pMessage << '\n';
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        break;
    }

    return VK_FALSE;
}
#endif // USE_VALIDATION_LAYERS

/// Check if a physical device supports the extensions required by the library.
static bool has_extension_support(VkPhysicalDevice device) {
    uint32_t num_extensions;
    vkEnumerateDeviceExtensionProperties(
        device, nullptr, &num_extensions, nullptr);

    std::vector<VkExtensionProperties> available(num_extensions);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &num_extensions, 
        available.data());
    
    std::set<std::string> required(EXTENSIONS.begin(), EXTENSIONS.end());

    for (const auto& extension : available)
        required.erase(extension.extensionName);

    return required.empty();
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

VulkanContext::VulkanContext() {
    init_vulkan_instance();
    init_vulkan_physical_device();
    init_vulkan_logical_device();
    init_vulkan_commands();
    init_vma_allocator();
}

VulkanContext::~VulkanContext() {
    if (m_device != nullptr)
        vkDeviceWaitIdle(m_device);

    if (m_cmd != nullptr) {
        vkFreeCommandBuffers(m_device, m_pool, 1, &m_cmd);
        m_cmd = nullptr;
    }

    if (m_pool != nullptr) {
        vkDestroyCommandPool(m_device, m_pool, nullptr);
        m_pool = nullptr;
    }
    
    if (m_allocator != nullptr) {
        vmaDestroyAllocator(m_allocator);
        m_allocator = nullptr;
    }

    if (m_device != nullptr) {
        vkDestroyDevice(m_device, nullptr);
        m_device = nullptr;
    }

#ifdef USE_VALIDATION_LAYERS
    auto destroy_fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( 
        vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (destroy_fn) 
        destroy_fn(m_instance, m_msger, nullptr);
#endif // USE_VALIDATION_LAYERS

    if (m_instance != nullptr) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = nullptr;
    }
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

    create_info.enabledExtensionCount = EXTENSIONS.size();
    create_info.ppEnabledExtensionNames = EXTENSIONS.data();

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
    auto create_fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));

    if (create_fn) 
        create_fn(m_instance, &msger_info, nullptr, &m_msger);
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

        if (!has_extension_support(device)) {
            std::cout << "Skipping device " << props.deviceName 
                << ": missing required extensions.\n";
            continue;
        }

#ifdef USE_VERBOSE_LOGGING
        std::cout << "using physical device: " << props.deviceName << '\n';
#endif // USE_VERBOSE_LOGGING

        m_physical_device = device;
        break;
    }

    if (!m_physical_device)
        throw rt_error("no suitable physical device found.");   
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

void VulkanContext::init_vulkan_commands() {
    VkCommandPoolCreateInfo pool_info {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(m_device, &pool_info, nullptr, &m_pool));

    VkCommandBufferAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = m_pool;
    alloc_info.commandBufferCount = 1;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(vkAllocateCommandBuffers(m_device, &alloc_info, &m_cmd));
}

void VulkanContext::init_vma_allocator() {
    VmaAllocatorCreateInfo info {};
    info.physicalDevice = m_physical_device;
    info.device = m_device;
    info.instance = m_instance;
    info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    VmaVulkanFunctions funcs {};
    funcs.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    funcs.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    info.pVulkanFunctions = &funcs;

    VK_CHECK(vmaCreateAllocator(&info, &m_allocator));
}

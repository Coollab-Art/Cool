#if defined(COOL_VULKAN)

#include "Context.h"
#include <Cool/Utils/Version.h>
#include <GLFW/glfw3.h>
#include "internal/check_result.h"

namespace Cool::Vulkan {
#if DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT /*flags*/, VkDebugReportObjectTypeEXT /*objectType*/, uint64_t /*object*/, size_t /*location*/, int32_t /*messageCode*/, const char* /*pLayerPrefix*/, const char* pMessage, void* /*pUserData*/)
{
    Log::internal_warning("Vulkan", pMessage);
    return VK_FALSE;
}
#endif

Context::Context()
{
    uint32_t     extensions_count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
    VkResult     err;
    // Create Vulkan Instance
    {
        VkInstanceCreateInfo create_info    = {};
        create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.enabledExtensionCount   = extensions_count;
        create_info.ppEnabledExtensionNames = extensions;
        VkApplicationInfo app_info          = {};
        app_info.sType                      = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.apiVersion                 = VK_MAKE_VERSION(major_version(COOL_VULKAN_VERSION), minor_version(COOL_VULKAN_VERSION), 0),
        create_info.pApplicationInfo        = &app_info;
#if DEBUG
        // Enabling validation layers
        const char* layers[]            = {"VK_LAYER_KHRONOS_validation"};
        create_info.enabledLayerCount   = 1;
        create_info.ppEnabledLayerNames = layers;

        // Enable debug report extension (we need additional storage, so we duplicate the user array to add our new extension to it)
        const char** extensions_ext = (const char**)malloc(sizeof(const char*) * (extensions_count + 1));
        memcpy(extensions_ext, extensions, extensions_count * sizeof(const char*));
        extensions_ext[extensions_count]    = "VK_EXT_debug_report";
        create_info.enabledExtensionCount   = extensions_count + 1;
        create_info.ppEnabledExtensionNames = extensions_ext;

        // Create Vulkan Instance
        err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
        check_result(err);
        free(extensions_ext);

        // Get the function pointer (required for any extensions)
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkCreateDebugReportCallbackEXT");
        IM_ASSERT(vkCreateDebugReportCallbackEXT != NULL);

        // Setup the debug report callback
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType                              = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags                              = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_report_ci.pfnCallback                        = debug_report;
        debug_report_ci.pUserData                          = NULL;
        err                                                = vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci, g_Allocator, &g_DebugReport);
        check_result(err);
#else
        // Create Vulkan Instance without any debug feature
        err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
        check_result(err);
        IM_UNUSED(g_DebugReport);
#endif
    }

    // Select GPU
    {
        uint32_t gpu_count;
        err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, NULL);
        check_result(err);
        IM_ASSERT(gpu_count > 0);

        VkPhysicalDevice* gpus = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
        err                    = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus);
        check_result(err);

        // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
        // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
        // dedicated GPUs) is out of scope of this sample.
        int use_gpu = 0;
        for (int i = 0; i < (int)gpu_count; i++)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(gpus[i], &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                use_gpu = i;
                break;
            }
        }

        g_PhysicalDevice = gpus[use_gpu];
        free(gpus);
    }

    // Select graphics queue family
    {
        uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, NULL);
        VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
        vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
        for (uint32_t i = 0; i < count; i++)
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                g_QueueFamily = i;
                break;
            }
        free(queues);
        IM_ASSERT(g_QueueFamily != (uint32_t)-1);
    }

    // Create Logical Device (with 1 queue)
    {
        int                     device_extension_count = 1;
        const char*             device_extensions[]    = {"VK_KHR_swapchain"};
        const float             queue_priority[]       = {1.0f};
        VkDeviceQueueCreateInfo queue_info[1]          = {};
        queue_info[0].sType                            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex                 = g_QueueFamily;
        queue_info[0].queueCount                       = 1;
        queue_info[0].pQueuePriorities                 = queue_priority;
        VkDeviceCreateInfo create_info                 = {};
        create_info.sType                              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount               = sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos                  = queue_info;
        create_info.enabledExtensionCount              = device_extension_count;
        create_info.ppEnabledExtensionNames            = device_extensions;
        err                                            = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
        check_result(err);
        vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
    }

    // Create Descriptor Pool
    {
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets                    = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount              = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes                 = pool_sizes;
        err                                  = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
        check_result(err);
    }

    // Get memory properties
    vkGetPhysicalDeviceMemoryProperties(g_PhysicalDevice, &memory_properties);

    // Create command pool
    {
        VkCommandPoolCreateInfo info = {};
        info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        info.queueFamilyIndex        = g_QueueFamily;
        err                          = vkCreateCommandPool(g_Device, &info, g_Allocator, &command_pool);
        check_result(err);
    }
}

void Context::destroy0()
{
    VkResult err = vkDeviceWaitIdle(g_Device);
    check_result(err);
}

void Context::destroy1()
{
    vkDestroyCommandPool(g_Device, command_pool, g_Allocator);
    vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);
#if DEBUG
    // Remove the debug report callback
    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif
    vkDestroyDevice(g_Device, g_Allocator);
    vkDestroyInstance(g_Instance, g_Allocator);
}

} // namespace Cool::Vulkan

#endif
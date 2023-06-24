#include <mspch.h>

#include "vkInstance.h"
#include "UtilsVulkan.h"

namespace ms
{
	vkInstance::vkInstance()
	{
        const VkApplicationInfo appInfo =
        {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Vulkan",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_3
        };

        ASSERT(AreLayersSupported());

        const VkInstanceCreateInfo createInfo =
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(VK_LAYERS.size()),
            .ppEnabledLayerNames = VK_LAYERS.data(),
            .enabledExtensionCount = static_cast<uint32_t>(VK_EXTENSIONS.size()),
            .ppEnabledExtensionNames = VK_EXTENSIONS.data()
        };

        VK_CHECK(vkCreateInstance(&createInfo, nullptr, &mVulkanInstance));

        volkLoadInstance(mVulkanInstance);

        SetupDebugCallbaks();
	}

	vkInstance::~vkInstance()
	{
        vkDestroySurfaceKHR(mVulkanInstance, mSurfaceKHR, nullptr);

        vkDestroyDebugReportCallbackEXT(mVulkanInstance, mReportCallbackEXT, nullptr);
        vkDestroyDebugUtilsMessengerEXT(mVulkanInstance, mMessengerEXT, nullptr);

        vkDestroyInstance(mVulkanInstance, nullptr);
	}

    bool vkInstance::AreLayersSupported()
    {
        uint32_t instanceLayerCount; // Stores number of layers supported by instance
        std::vector<VkLayerProperties> layerProperties; // Vector to store layer properties

        VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, NULL));
        layerProperties.resize(instanceLayerCount);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, layerProperties.data()));

        for (size_t i = 0; i < VK_LAYERS.size(); i++)
        {
            VkBool32 isSupported = 0;

            for (size_t j = 0; j < layerProperties.size(); j++)
            {
                VkLayerProperties layerProp = layerProperties[j];

                if (!strcmp(VK_LAYERS[i], layerProp.layerName))
                {
                    isSupported = 1;
                    break;
                }
            }

            if (!isSupported)
            {
                std::cout << "No Layer support found, removed from layer: " << VK_LAYERS[i] << std::endl << std::endl;
                return false;
            }
            else
            {
                std::cout << "Layer supported: " << VK_LAYERS[i] << std::endl << std::endl;
            }
        }

        return true;
    }

    void vkInstance::SetupDebugCallbaks()
    {
        const VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = &_VulkanDebugCallback,
            .pUserData = nullptr
        };

        VK_CHECK(vkCreateDebugUtilsMessengerEXT(mVulkanInstance, &messengerCreateInfo, nullptr, &mMessengerEXT));

        const VkDebugReportCallbackCreateInfoEXT messengerCreateInfo2 =
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
                | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT,
            .pfnCallback = &_VulkanDebugReportCallback,
            .pUserData = nullptr
        };

        VK_CHECK(vkCreateDebugReportCallbackEXT(mVulkanInstance, &messengerCreateInfo2, nullptr, &mReportCallbackEXT));
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL vkInstance::_VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
    {
        std::cout << "[VK_VALIDATION_LAYER]:" << callbackData->pMessage << std::endl << std::endl;

        fflush(stdout);
        return VK_TRUE;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL vkInstance::_VulkanDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* userData)
    {
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
            std::cout << "[VK_DEBUG_REPORT] ERROR: [" << pLayerPrefix << "] Code" << messageCode << ":" << pMessage << std::endl << std::endl;
        }
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
            std::cout << "[VK_DEBUG_REPORT] WARNING: [" << pLayerPrefix << "] Code" << messageCode << ":" << pMessage << std::endl << std::endl;
        }
        else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
            std::cout << "[VK_DEBUG_REPORT] INFORMATION: [" << pLayerPrefix << "] Code" << messageCode << ":" << pMessage << std::endl << std::endl;
        }
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
            //std::cout << "[VK_DEBUG_REPORT] PERFORMANCE: [" << pLayerPrefix << "] Code" << messageCode << ":" << pMessage << std::endl << std::endl;
            return VK_FALSE;
        }
        else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
            std::cout << "[VK_DEBUG_REPORT] DEBUG: [" << pLayerPrefix << "] Code" << messageCode << ":" << pMessage << std::endl << std::endl;
        }
        else {
            return VK_FALSE;
        }

        fflush(stdout);
        return VK_TRUE;
    }
}
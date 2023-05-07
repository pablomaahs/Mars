#pragma once

#include <string.h>

#include "Include/glslang_c_interface.h"
#include "ResourceLimits.h"
#include "platform/common/utils/Utils.h"

#include "volk.h"

#define VK_CHECK(value)\
    if (value != VK_SUCCESS)\
    {\
        CHECK(false, __FILE__, __LINE__);\
    }

#define VK_CHECK_RET(value)\
    if (value != VK_SUCCESS)\
    {\
        CHECK(false, __FILE__, __LINE__);\
    }\
    return true;\

namespace ms
{
    struct ShaderModule final
    {
        std::vector<unsigned int> SPIRV;
        VkShaderModule shaderModule;
    };

    struct SwapChainSupportDetails final
    {
        VkSurfaceCapabilitiesKHR capabilities = {};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
}

#define VK_CHECK(value)\
    if (value != VK_SUCCESS)\
    {\
        VK_ASSERT(false);\
    }\
    return false;\

#define VK_CHECK_RET(value)\
    if (value != VK_SUCCESS)\
    {\
        VK_ASSERT(false);\
    }\
    return value;\

static void VK_ASSERT(bool check)
{
    if (!check)
    {
        exit(EXIT_FAILURE);
    }
}

size_t compileShader(glslang_stage_t stage, const char* shaderSource, ms::ShaderModule& shaderModule);

size_t compileShaderFile(const char* file, glslang_stage_t stage, ms::ShaderModule& shaderModule);

void saveSPIRVBinaryFile(const char* filename, unsigned int* code, size_t size);

void createInstance(VkInstance& instance);

VkResult createDevice(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures deviceFeatures, uint32_t graphicsFamily, VkDevice* device);

VkResult findSuitablePhysicalDevice(VkInstance instance, std::function<bool(VkPhysicalDevice)> selector, VkPhysicalDevice* physicalDevice);

uint32_t findQueueFamilies(VkPhysicalDevice physicalDevice, VkQueueFlags desiredFlags);

ms::SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

uint32_t chooseSwapImageCount(const VkSurfaceCapabilitiesKHR& capabilities);

VkResult createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t graphicsFamily, uint32_t width, uint32_t height, VkSwapchainKHR* swapchain, bool supportScreenshots);

size_t createSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage>& swapchainImages, std::vector<VkImageView>& swapchainImageViews);

bool createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, uint32_t layerCount = 1, uint32_t mipLevels = 1);
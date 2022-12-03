#include <mspch.h>

#include "UtilsVulkan.h"
#include "platform/common/utils/Utils.h"

size_t compileShader(glslang_stage_t stage, const char* shaderSource, ms::ShaderModule& shaderModule)
{
    const glslang_input_t input = {
        .language = GLSLANG_SOURCE_GLSL,
        .stage = stage,
        .client = GLSLANG_CLIENT_VULKAN,
        .client_version = GLSLANG_TARGET_VULKAN_1_1,
        .target_language = GLSLANG_TARGET_SPV,
        .target_language_version = GLSLANG_TARGET_SPV_1_1,
        .code = shaderSource,
        .default_version = 100,
        .default_profile = GLSLANG_NO_PROFILE,
        .force_default_version_and_profile = false,
        .forward_compatible = false,
        .messages = GLSLANG_MSG_DEFAULT_BIT,
        .resource = (const glslang_resource_t*)&glslang::DefaultTBuiltInResource,
    };

    glslang_shader_t* shader = glslang_shader_create(&input);

    if (!glslang_shader_preprocess(shader, &input))
    {
        std::cerr << glslang_shader_get_info_log(shader) << std::endl << glslang_shader_get_info_debug_log(shader);
        throw std::runtime_error("GLSL preprocessing failed\n");
        return -1;
    }

    if (!glslang_shader_parse(shader, &input))
    {
        std::cerr << glslang_shader_get_info_log(shader) << std::endl << glslang_shader_get_info_debug_log(shader);
        throw std::runtime_error("GLSL parse failed\n");
        return -1;
    }

    glslang_program_t* program = glslang_program_create();
    glslang_program_add_shader(program, shader);
    int msgs = GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT;

    if (!glslang_program_link(program, msgs))
    {
        std::cerr << glslang_shader_get_info_log(shader) << std::endl << glslang_shader_get_info_debug_log(shader);
        throw std::runtime_error("GLSL linking failed\n");
        return -1;
    }

    glslang_program_SPIRV_generate(program, stage);
    shaderModule.SPIRV.resize(glslang_program_SPIRV_get_size(program));
    glslang_program_SPIRV_get(program, shaderModule.SPIRV.data());
    const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
    if (spirv_messages)
    {
        std::cout << spirv_messages << std::endl;
    }

    glslang_program_delete(program);
    glslang_shader_delete(shader);

    return shaderModule.SPIRV.size();
}

size_t compileShaderFile(const char* file, glslang_stage_t stage, ms::ShaderModule& shaderModule)
{
    if (auto shaderSource = readShaderFile(file); !shaderSource.empty())
    {
        return compileShader(stage, shaderSource.c_str(), shaderModule);
    }

    return -1;
}

void saveSPIRVBinaryFile(const char* filename, unsigned int* code, size_t size)
{
    FILE* f = fopen(filename, "w");

    if (!f)
        return;

    fwrite(code, sizeof(uint32_t), size, f);
    fclose(f);
}

void createInstance(VkInstance& instance)
{
    const std::vector<const char*> layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined (_WIN32)
        "VK_KHR_win32_surface"
#endif // WIN32
    };

    const VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Vulkan",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3
    };

    const VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };

    VK_ASSERT(volkInitialize() == VK_SUCCESS);
    VK_ASSERT(vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS);

    volkLoadInstance(instance);
}

VkResult createDevice(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures deviceFeatures, uint32_t graphicsFamily, VkDevice* device)
{
    const std::vector<const char*> extensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    const float queuePriority = 1.0f;
    const VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = graphicsFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    const VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
        .pEnabledFeatures = &deviceFeatures
    };

    VK_CHECK_RET(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, device));
}

VkResult findSuitablePhysicalDevice(VkInstance instance, std::function<bool(VkPhysicalDevice)> selector, VkPhysicalDevice* physicalDevice)
{
    uint32_t deviceCount = 0;

    VK_CHECK_RET(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHECK_RET(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));
    for (const auto& device : devices)
    {
        if (selector(device))
        {
            *physicalDevice = device;
            return VK_SUCCESS;
        }
    }

    return VK_ERROR_INITIALIZATION_FAILED;
}

uint32_t findQueueFamilies(VkPhysicalDevice physicalDevice, VkQueueFlags desiredFlags)
{
    uint32_t familyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> families(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.data());
    for (uint32_t i = 0; i != families.size(); i++)
    {
        if (families[i].queueCount > 0 && families[0].queueFlags & desiredFlags)
        {
            return i;
        }
    }

    return 0;
}

ms::SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    ms::SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

    if (formatCount)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

    if (presentModeCount)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto mode : availablePresentModes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

uint32_t chooseSwapImageCount(const VkSurfaceCapabilitiesKHR& capabilities)
{
    const uint32_t imageCount = capabilities.minImageCount + 1;

    const bool imageCountExceeded = capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount;

    return imageCountExceeded ? capabilities.maxImageCount : imageCount;
}

VkResult createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t graphicsFamily, uint32_t width, uint32_t height, VkSwapchainKHR* swapchain, bool supportScreenshots)
{
    auto swapchainSupport = querySwapchainSupport(physicalDevice, surface);
    auto surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
    auto presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);

    const VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface,
        .minImageCount = chooseSwapImageCount(swapchainSupport.capabilities),
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = {.width = width, .height = height},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | (supportScreenshots ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0u),
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &graphicsFamily,
        .preTransform = swapchainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    VK_CHECK_RET(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, swapchain));
}

size_t createSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage>& swapchainImages, std::vector<VkImageView>& swapchainImageViews)
{
    uint32_t imageCount = 0u;
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));

    swapchainImages.resize(imageCount);
    swapchainImageViews.resize(imageCount);

    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data()));

    for (unsigned i = 0; i < imageCount; i++)
    {
        if (!createImageView(device, swapchainImages[i], VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &swapchainImageViews[i]))
        {
            exit(EXIT_FAILURE);
        }
    }

    return static_cast<size_t>(imageCount);
}

bool createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView, VkImageViewType viewType, uint32_t layerCount, uint32_t mipLevels)
{
    const VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = image,
        .viewType = viewType,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = layerCount
        }
    };

    VK_CHECK_RET(vkCreateImageView(device, &imageViewCreateInfo, nullptr, imageView));
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    return { VK_FORMAT_R8G8B8A8_UNORM , VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
}

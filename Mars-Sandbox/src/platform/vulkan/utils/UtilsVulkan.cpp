#include <mspch.h>

#include "UtilsVulkan.h"
#include "platform/common/utils/Utils.h"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/version.h"

#pragma warning(disable : 26451)
#include "stb/stb_image.h"

// Vulkan Instance related objects
void CreateVulkanInstance(VkInstance* instance)
{
    const std::vector<const char*> layers =
    {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> extensions =
    {
        VK_KHR_SURFACE_EXTENSION_NAME
       ,VK_EXT_DEBUG_UTILS_EXTENSION_NAME
       ,VK_EXT_DEBUG_REPORT_EXTENSION_NAME
        /* for indexed textures */
       ,VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
#if defined (_WIN32)
       ,"VK_KHR_win32_surface"
#endif // WIN32
    };

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

    const VkInstanceCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, instance));

    volkLoadInstance(*instance);
}

#pragma region 1 Vulkan Instance

// Set up Vulkan Debugging capabilities

bool SetupDebugCallbaks(VkInstance* instance, VkDebugUtilsMessengerEXT* messenger, VkDebugReportCallbackEXT* reportCallback)
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

    VK_CHECK(vkCreateDebugUtilsMessengerEXT(*instance, &messengerCreateInfo, nullptr, messenger));

    const VkDebugReportCallbackCreateInfoEXT messengerCreateInfo2 =
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags =
            VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT,
        .pfnCallback = &_VulkanDebugReportCallback,
        .pUserData = nullptr
    };

    VK_CHECK_RET(vkCreateDebugReportCallbackEXT(*instance, &messengerCreateInfo2, nullptr, reportCallback));
}

VKAPI_ATTR VkBool32 VKAPI_CALL _VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT secerity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* calbackData, void* userData)
{
    printf("Validation layer: %s\n\n", calbackData->pMessage);
    return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL _VulkanDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* userData)
{
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        return VK_FALSE;
    printf("Debug callback (%s): %s\n\n", pLayerPrefix, pMessage);
    return VK_FALSE;
}

#pragma endregion

// Vulkan Render Device related objects
bool CreateVulkanRenderDevice(ms::VulkanInstance* instance, ms::VulkanRenderDevice* renderDevice, uint32_t width, uint32_t height, std::function<bool(VkPhysicalDevice)> selector, VkPhysicalDeviceFeatures deviceFeatures)
{
    renderDevice->framebufferHeight = height;
    renderDevice->framebufferWidth = width;

    if (!_FindSuitablePhysicalDevice(instance->instance, selector, &renderDevice->physicalDevice))
    {
        ASSERT(false);
    }

    renderDevice->graphicsFamily = _FindQueueFamilies(renderDevice->physicalDevice, VK_QUEUE_GRAPHICS_BIT);

    if (!_CreateDevice(renderDevice->physicalDevice, deviceFeatures, renderDevice->graphicsFamily, &renderDevice->device))
    {
        ASSERT(false);
    }

    vkGetDeviceQueue(renderDevice->device, renderDevice->graphicsFamily, 0, &renderDevice->graphicsQueue);
    ASSERT(renderDevice->graphicsQueue != nullptr);

    VkBool32 presentSupported = 0;
    vkGetPhysicalDeviceSurfaceSupportKHR(renderDevice->physicalDevice, renderDevice->graphicsFamily, instance->surface, &presentSupported);
    ASSERT(presentSupported != 0);

    ASSERT(_CreateSwapchain(renderDevice->device, renderDevice->physicalDevice, instance->surface, renderDevice->graphicsFamily, width, height, &renderDevice->swapchain, true));

    size_t imageCount = 0;
    imageCount = _CreateSwapchainImages(renderDevice->device, renderDevice->swapchain, renderDevice->swapchainImages, renderDevice->swapchainImageViews);
    ASSERT(imageCount != 0);

    if (!_CreateSemaphore(renderDevice->device, &renderDevice->semaphore))
    {
        ASSERT(false);
    }

    if (!_CreateSemaphore(renderDevice->device, &renderDevice->renderSemaphore))
    {
        ASSERT(false);
    }

    const VkCommandPoolCreateInfo commandPoolCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = 0,
        .queueFamilyIndex = renderDevice->graphicsFamily
    };

    VK_CHECK(vkCreateCommandPool(renderDevice->device, &commandPoolCreateInfo, nullptr, &renderDevice->commandPool));

    const VkCommandBufferAllocateInfo commandPoolAllocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = renderDevice->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = (uint32_t)(renderDevice->swapchainImages.size())
    };

    renderDevice->commandBuffers.resize(imageCount);
    VK_CHECK(vkAllocateCommandBuffers(renderDevice->device, &commandPoolAllocateInfo, &renderDevice->commandBuffers[0]));

    return true;
}

#pragma region 2 Vulkan Render Device

// Create VkDevice
bool _FindSuitablePhysicalDevice(VkInstance instance, std::function<bool(VkPhysicalDevice)> selector, VkPhysicalDevice* physicalDevice)
{
    uint32_t deviceCount = 0;

    VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
    std::vector<VkPhysicalDevice> devices(deviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));
    for (const auto& device : devices)
    {
        if (selector(device))
        {
            *physicalDevice = device;
            return true;
        }
    }

    return false; // VK_ERROR_INITIALIZATION_FAILED
}

uint32_t _FindQueueFamilies(VkPhysicalDevice physicalDevice, VkQueueFlags desiredFlags)
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

bool _CreateDevice(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures deviceFeatures, uint32_t graphicsFamily, VkDevice* device)
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

// Create VkSwapchain
bool _CreateSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t graphicsFamily, uint32_t width, uint32_t height, VkSwapchainKHR* swapchain, bool supportScreenshots)
{
    auto swapchainSupport = _QuerySwapchainSupport(physicalDevice, surface);
    auto surfaceFormat = _ChooseSwapSurfaceFormat(swapchainSupport.formats);
    auto presentMode = _ChooseSwapPresentMode(swapchainSupport.presentModes);

    const VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface,
        .minImageCount = _ChooseSwapImageCount(swapchainSupport.capabilities),
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

ms::SwapChainSupportDetails _QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
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

VkSurfaceFormatKHR _ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    VkSurfaceFormatKHR targetFormat{ VK_FORMAT_R8G8B8A8_UNORM , VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    bool found = false;

    for (const auto format : formats)
    {
        if (format.format == targetFormat.format)
        {
            if (format.colorSpace == targetFormat.colorSpace)
            {
                found = true;
                break;
            }
        }
    }

    ASSERT(found);

    return targetFormat;
}

VkPresentModeKHR _ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    VkPresentModeKHR targetPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    VkPresentModeKHR defaultPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto mode : availablePresentModes)
    {
        if (mode == targetPresentMode)
        {
            return targetPresentMode;
        }
    }

    return defaultPresentMode;
}

uint32_t _ChooseSwapImageCount(const VkSurfaceCapabilitiesKHR& capabilities)
{
    const uint32_t imageCount = capabilities.minImageCount + 1;

    const bool imageCountExceeded = capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount;

    return imageCountExceeded ? capabilities.maxImageCount : imageCount;
}

// Create VkImages and VkImageViews
size_t _CreateSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage>& swapchainImages, std::vector<VkImageView>& swapchainImageViews)
{
    uint32_t imageCount = 0u;
    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));

    swapchainImages.resize(imageCount);
    swapchainImageViews.resize(imageCount);

    VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data()));

    for (unsigned i = 0; i < imageCount; i++)
    {
        if (!CreateImageView(device, swapchainImages[i], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &swapchainImageViews[i]))
        {
            exit(EXIT_FAILURE);
        }
    }

    return static_cast<size_t>(imageCount);
}

bool CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView, VkImageViewType viewType, uint32_t layerCount, uint32_t mipLevels)
{
    const VkImageViewCreateInfo imageViewCreateInfo =
    {
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

// Create VkSemaphore
bool _CreateSemaphore(VkDevice device, VkSemaphore* outSemaphore)
{
    const VkSemaphoreCreateInfo semaphoreCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VK_CHECK_RET(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, outSemaphore));
}

#pragma endregion

bool CreateTexturedVertexBuffer(const ms::VulkanRenderDevice& renderDevice, const char* fileName, VkBuffer* storageBuffer, VkDeviceMemory* storageBufferMemory, size_t* vertexBufferSize, size_t* indexBufferSize)
{
    // TODO: Move Assimp Import Model Code elsewhere
    const aiScene* scene = aiImportFile(fileName, aiProcess_Triangulate);
    ASSERT(scene != nullptr);

    if (!scene || !scene->HasMeshes())
    {
        std::cout << "Model has no Mesh" << std::endl;
        ASSERT(false);
        exit(99);
    }

    const aiMesh* mesh = scene->mMeshes[0];
    struct VertexData {
        glm::vec3 pos;
        glm::vec2 tc;
    };

    std::vector<VertexData> vertices;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        const aiVector3D v = mesh->mVertices[i];
        const aiVector3D t = mesh->mTextureCoords[0][i];
        vertices.push_back({ glm::vec3(v.x, v.y, v.z), glm::vec2(t.x, t.y) });
    }

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        for (unsigned int j = 0; j < 3; j++)
        {
            indices.push_back({ mesh->mFaces[i].mIndices[j] });
        }
    }

    aiReleaseImport(scene);

    *vertexBufferSize = sizeof(VertexData) * vertices.size();
    *indexBufferSize = sizeof(unsigned int) * indices.size();
    renderDevice.indexBufferSize = *indexBufferSize;

    VkDeviceSize bufferSize = *vertexBufferSize + *indexBufferSize;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    ASSERT(
        CreateBuffer(
            renderDevice.device, renderDevice.physicalDevice, bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingMemory
        )
    );

    void* data;
    VK_CHECK(vkMapMemory(renderDevice.device, stagingMemory, 0, bufferSize, 0, &data));
    memcpy(data, vertices.data(), *vertexBufferSize);
    memcpy((unsigned char*)data + *vertexBufferSize, indices.data(), *indexBufferSize);
    vkUnmapMemory(renderDevice.device, stagingMemory);

    ASSERT(
        CreateBuffer(
            renderDevice.device, renderDevice.physicalDevice, bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            *storageBuffer, *storageBufferMemory
        )
    );

    _CopyBuffer(renderDevice, stagingBuffer, *storageBuffer, bufferSize);
    vkDestroyBuffer(renderDevice.device, stagingBuffer, nullptr);
    vkFreeMemory(renderDevice.device, stagingMemory, nullptr);

    return true;
}

#pragma region 3 Textured Vertex Buffer

// Create the GPU Buffer
bool CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    const VkBufferCreateInfo bufferInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };

    VK_CHECK(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = _FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, properties)
    };

    VK_CHECK(vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory));
    VK_CHECK_RET(vkBindBufferMemory(device, buffer, bufferMemory, 0));
}

bool CreateUniformBuffers(ms::VulkanState& vkState, ms::VulkanRenderDevice& rendererDevice)
{
    VkDeviceSize bufferSize = sizeof(ms::UniformBuffer);
    vkState.uniformBuffers.resize(rendererDevice.swapchainImages.size());
    vkState.uniformBuffersMemory.resize(rendererDevice.swapchainImages.size());

    for (size_t i = 0; i < rendererDevice.swapchainImages.size(); i++)
    {
        if (
            !CreateBuffer(rendererDevice.device, rendererDevice.physicalDevice, bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                vkState.uniformBuffers[i], vkState.uniformBuffersMemory[i]
            )
            )
        {
            ASSERT(false);
            return false;
        }
    }

    return true;
}

uint32_t _FindMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    return 0xFFFFFFFF;
}

void _CopyBuffer(const ms::VulkanRenderDevice & renderDevice, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer;

    // Begin Single time Command Buffer
    const VkCommandBufferAllocateInfo allocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = renderDevice.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VK_CHECK(vkAllocateCommandBuffers(renderDevice.device, &allocateInfo, &commandBuffer));

    const VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    const VkBufferCopy copyParam =
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyParam);

    // End Single time Command Buffer
    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    const VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr
    };

    VK_CHECK(vkQueueSubmit(renderDevice.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK(vkQueueWaitIdle(renderDevice.graphicsQueue));
    vkFreeCommandBuffers(renderDevice.device, renderDevice.commandPool, 1, &commandBuffer);
}

#pragma endregion

// Create VkImage
bool CreateTextureImage(const ms::VulkanRenderDevice & device, const char* fileName, VkImage& textureImage, VkDeviceMemory& textureImageMemory)
{
    int texWidth, texHeight, texChannels;

    stbi_uc* pixels = stbi_load(fileName, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        std::cout << "Failed to load " << fileName << " texture" << std::endl;
        ASSERT(false);
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    ASSERT(
        CreateBuffer(
            device.device, device.physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingMemory
        )
    );

    void* data;
    VK_CHECK(vkMapMemory(device.device, stagingMemory, 0, imageSize, 0, &data));
    #pragma warning(suppress : 6387)
    memcpy(data, pixels, static_cast<size_t>(imageSize));

    vkUnmapMemory(device.device, stagingMemory);

    ASSERT(
        _CreateImage(
            device.device, device.physicalDevice, texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory
        )
    );

    _TransitionImageLayout(device, textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    _CopyBufferToImage(device, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    _TransitionImageLayout(device, textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device.device, stagingBuffer, nullptr);
    vkFreeMemory(device.device, stagingMemory, nullptr);

    stbi_image_free(pixels);
    return true;
}

// Create VkSampler
bool CreateTextureSampler(VkDevice device, VkSampler* sampler)
{
    const VkSamplerCreateInfo samplerInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    VK_CHECK_RET(vkCreateSampler(device, &samplerInfo, nullptr, sampler));
}

#pragma region 4 Texture Image and Texture Sampler

bool _CreateImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    const VkImageCreateInfo imageCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = VkExtent3D {
            .width = width, .height = height, .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount  = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VK_CHECK(vkCreateImage(device, &imageCreateInfo, nullptr, &image));
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);

    const VkMemoryAllocateInfo memoryAllocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = _FindMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, properties)
    };

    VK_CHECK(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &imageMemory));
    vkBindImageMemory(device, image, imageMemory, 0);

    return true;
}

void _TransitionImageLayout(const ms::VulkanRenderDevice & renderDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount/* = 1*/, uint32_t mipLevels/* = 1*/)
{
    VkCommandBuffer commandBuffer;

    const VkCommandBufferAllocateInfo allocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = renderDevice.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VK_CHECK(vkAllocateCommandBuffers(renderDevice.device, &allocateInfo, &commandBuffer));

    const VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    _TransitionImageLayoutCmd(commandBuffer, image, format, oldLayout, newLayout, layerCount, mipLevels);
    
    // End Single time Command Buffer
    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    const VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr
    };

    VK_CHECK(vkQueueSubmit(renderDevice.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK(vkQueueWaitIdle(renderDevice.graphicsQueue));
    vkFreeCommandBuffers(renderDevice.device, renderDevice.commandPool, 1, &commandBuffer);
}

void _TransitionImageLayoutCmd(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount, uint32_t mipLevels)
{
    VkImageMemoryBarrier barrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = VkImageSubresourceRange {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    VkPipelineStageFlags sourceStage, destinationStage;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (_HasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask   = 0;
        barrier.dstAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage             = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage        = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
        sourceStage             = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage        = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask   = 0;
        barrier.dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage             = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage        = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

bool _HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void _CopyBufferToImage(ms::VulkanRenderDevice renderDevice, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer;

    const VkCommandBufferAllocateInfo allocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = renderDevice.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VK_CHECK(vkAllocateCommandBuffers(renderDevice.device, &allocateInfo, &commandBuffer));

    const VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    const VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = VkImageSubresourceLayers {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = VkOffset3D { .x = 0, .y = 0, .z = 0 },
        .imageExtent = VkExtent3D {
            .width = width, .height = height, .depth = 1
        }
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // End Single time Command Buffer
    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    const VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr
    };

    VK_CHECK(vkQueueSubmit(renderDevice.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK(vkQueueWaitIdle(renderDevice.graphicsQueue));
    vkFreeCommandBuffers(renderDevice.device, renderDevice.commandPool, 1, &commandBuffer);
}

#pragma endregion

bool CreateDepthResources(const ms::VulkanRenderDevice & renderDevice, uint32_t width, uint32_t height, ms::VulkanImage& depth)
{
    VkFormat depthFormat = _FindDepthFormat(renderDevice.physicalDevice);
    ASSERT(
        _CreateImage(renderDevice.device, renderDevice.physicalDevice, width, height, depthFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            depth.image, depth.imageMemory
        )
    );
    ASSERT(CreateImageView(renderDevice.device, depth.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &depth.imageView));
    _TransitionImageLayout(renderDevice, depth.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    return true;
}

#pragma region 5 Depth Resources

VkFormat _FindDepthFormat(VkPhysicalDevice device)
{
    return findSupportedFormat(device, 
        {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

#pragma endregion

bool CreateDescriptorPool(VkDevice device, uint32_t imageCount, uint32_t uniformBufferCount, uint32_t storageBufferCount, uint32_t samplerCount, VkDescriptorPool* descPool)
{
    std::vector<VkDescriptorPoolSize> poolSizes;

    if (uniformBufferCount)
    {
        poolSizes.push_back(VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = imageCount * uniformBufferCount});
    }

    if (storageBufferCount)
    {
        poolSizes.push_back(VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = imageCount * storageBufferCount});
    }

    if (samplerCount)
    {
        poolSizes.push_back(VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = imageCount * samplerCount});
    }

    const VkDescriptorPoolCreateInfo descPoolCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = static_cast<uint32_t>(imageCount),
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.empty() ? nullptr : poolSizes.data()
    };

    VK_CHECK_RET(vkCreateDescriptorPool(device, &descPoolCreateInfo, nullptr, descPool));
}

bool CreateDescriptorSet(ms::VulkanRenderDevice& renderDevice, ms::VulkanState& state, uint32_t vertexBufferSize, uint32_t indexBufferSize)
{
    const std::vector<VkDescriptorSetLayoutBinding> bindings
    {
        VkDescriptorSetLayoutBinding{
            .binding = static_cast<uint32_t>(0),
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr
        },
        VkDescriptorSetLayoutBinding{
            .binding = static_cast<uint32_t>(1),
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr
        },
        VkDescriptorSetLayoutBinding{
            .binding = static_cast<uint32_t>(2),
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr
        },
        VkDescriptorSetLayoutBinding{
            .binding = static_cast<uint32_t>(3),
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        }
    };

    const VkDescriptorSetLayoutCreateInfo descSetLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    VK_CHECK(vkCreateDescriptorSetLayout(renderDevice.device, &descSetLayoutCreateInfo, nullptr, &state.descriptorSetLayout));

    std::vector<VkDescriptorSetLayout> layouts = {
        renderDevice.swapchainImages.size(),
        state.descriptorSetLayout
    };

    VkDescriptorSetAllocateInfo descSetAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = state.descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(renderDevice.swapchainImages.size()),
        .pSetLayouts = layouts.data()
    };

    state.descriptorSets.resize(renderDevice.swapchainImages.size());
    VK_CHECK(vkAllocateDescriptorSets(renderDevice.device, &descSetAllocateInfo, state.descriptorSets.data()));

    for (size_t i = 0; i < renderDevice.swapchainImages.size(); i++)
    {
        const VkDescriptorBufferInfo bufferInfo = {
            .buffer = state.uniformBuffers[i],
            .offset = 0,
            .range = sizeof(ms::UniformBuffer)
        };

        const VkDescriptorBufferInfo bufferInfo2 = {
            .buffer = state.storageBuffer,
            .offset = 0,
            .range = vertexBufferSize
        };

        const VkDescriptorBufferInfo bufferInfo3 = {
            .buffer = state.storageBuffer,
            .offset = vertexBufferSize,
            .range = indexBufferSize
        };

        const VkDescriptorImageInfo imageInfo = {
            .sampler = state.textureSampler,
            .imageView = state.texture.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        std::vector<VkWriteDescriptorSet> descriptorWrites = {
            VkWriteDescriptorSet
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = state.descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &bufferInfo
            },
            VkWriteDescriptorSet
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = state.descriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &bufferInfo2
            },
            VkWriteDescriptorSet
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = state.descriptorSets[i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &bufferInfo3
            },
            VkWriteDescriptorSet
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = state.descriptorSets[i],
                .dstBinding = 3,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &imageInfo
            }
        };

        vkUpdateDescriptorSets(renderDevice.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    return true;
}

bool CreateColorAndDepthRenderPass(const ms::VulkanRenderDevice& renderDevice, bool useDepth, VkRenderPass* renderPass, const ms::RenderPassCreateInfo& ci, VkFormat colorFormat)
{
    const bool offscreenInt = ci.flags_ & ms::eRenderPassBit_OffscreenInternal;
    const bool first = ci.flags_ & ms::eRenderPassBit_First;
    const bool last = ci.flags_ & ms::eRenderPassBit_Last;

    VkAttachmentDescription colorAttachment =
    {
        .flags = 0,
        .format = colorFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = offscreenInt ? VK_ATTACHMENT_LOAD_OP_LOAD : (ci.clearColor_ ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD),
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = first ? VK_IMAGE_LAYOUT_UNDEFINED : (offscreenInt ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
        .finalLayout = last ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    const VkAttachmentReference colorAttachmentRef =
    {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription depthAttachment =
    {
        .flags = 0,
        .format = useDepth ? _FindDepthFormat(renderDevice.physicalDevice) : VK_FORMAT_D32_SFLOAT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = offscreenInt ? VK_ATTACHMENT_LOAD_OP_LOAD : (ci.clearDepth_ ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD),
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = ci.clearDepth_ ? VK_IMAGE_LAYOUT_UNDEFINED : (offscreenInt ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    const VkAttachmentReference depthAttachmentRef =
    {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    if (ci.flags_ & ms::eRenderPassBit_Offscreen)
    {
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    std::vector<VkSubpassDependency> dependencies =
    {
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = 0
        }
    };

    if (ci.flags_ & ms::eRenderPassBit_Offscreen)
    {
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Use subpass dependencies for layout transitions
        dependencies.resize(2);

        dependencies[0] =
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        };

        dependencies[1] =
        {
            .srcSubpass = 0,
            .dstSubpass = VK_SUBPASS_EXTERNAL,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
        };
    }

    const VkSubpassDescription subpass =
    {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = useDepth ? &depthAttachmentRef : nullptr,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr
    };

    std::vector<VkAttachmentDescription> attachments =
    {
        colorAttachment,
        depthAttachment
    };

    const VkRenderPassCreateInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = static_cast<uint32_t>(useDepth ? 2 : 1),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = static_cast<uint32_t>(dependencies.size()),
        .pDependencies = dependencies.data()
    };

    VK_CHECK_RET(vkCreateRenderPass(renderDevice.device, &renderPassInfo, nullptr, renderPass));
}

bool CreatePipelineLayout(const ms::VulkanRenderDevice& renderDevice, VkDescriptorSetLayout dsLayout, VkPipelineLayout* pipelineLayout)
{
    const VkPipelineLayoutCreateInfo pipelineLayoutInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &dsLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    VK_CHECK_RET(vkCreatePipelineLayout(renderDevice.device, &pipelineLayoutInfo, nullptr, pipelineLayout));
}

bool CreateGraphicsPipeline(
    const ms::VulkanRenderDevice& renderDevice,
    VkRenderPass renderPass, VkPipelineLayout pipelineLayout, VkPipeline* graphicsPipeline,
    const std::vector<std::string>& shaderFiles,
    VkPrimitiveTopology topology,
    bool useDepth, bool useBlending, bool dynamicScissorState,
    int32_t customWidth, int32_t customHeight,
    uint32_t numPatchControlPoints
)
{
    std::vector <ms::ShaderModule> shaderModules{ 3 };
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};

    {
        _CreateShaderModule(renderDevice.device, &shaderModules[0], shaderFiles[0], GLSLANG_STAGE_VERTEX);

        VkPipelineShaderStageCreateInfo stage
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = shaderModules[0].shaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr
        };

        shaderStages.push_back(stage);
    }

    {
        _CreateShaderModule(renderDevice.device, &shaderModules[1], shaderFiles[1], GLSLANG_STAGE_FRAGMENT);

        VkPipelineShaderStageCreateInfo stage
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = shaderModules[1].shaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr
        };

        shaderStages.push_back(stage);
    }

    {
        _CreateShaderModule(renderDevice.device, &shaderModules[2], shaderFiles[2], GLSLANG_STAGE_GEOMETRY);

        VkPipelineShaderStageCreateInfo stage
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
            .module = shaderModules[2].shaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr
        };

        shaderStages.push_back(stage);
    }

    const VkPipelineVertexInputStateCreateInfo vertexInputInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };

    const VkPipelineInputAssemblyStateCreateInfo inputAssembly =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        /* The only difference from createGraphicsPipeline() */
        .topology = topology,
        .primitiveRestartEnable = VK_FALSE
    };

    const VkViewport viewport =
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(customWidth > 0 ? customWidth : renderDevice.framebufferWidth),
        .height = static_cast<float>(customHeight > 0 ? customHeight : renderDevice.framebufferHeight),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    const VkRect2D scissor =
    {
        .offset = { 0, 0 },
        .extent =
        {
            customWidth > 0 ? customWidth : renderDevice.framebufferWidth,
            customHeight > 0 ? customHeight : renderDevice.framebufferHeight
        }
    };

    const VkPipelineViewportStateCreateInfo viewportState =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    const VkPipelineRasterizationStateCreateInfo rasterizer =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .lineWidth = 1.0f
    };

    const VkPipelineMultisampleStateCreateInfo multisampling =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f
    };

    const VkPipelineColorBlendAttachmentState colorBlendAttachment =
    {
        .blendEnable = VK_FALSE,
        //.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        //.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        //.colorBlendOp = VK_BLEND_OP_ADD,
        //.srcAlphaBlendFactor = useBlending ? VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA : VK_BLEND_FACTOR_ONE,
        //.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        //.alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    const VkPipelineColorBlendStateCreateInfo colorBlending =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
    };

    const VkPipelineDepthStencilStateCreateInfo depthStencil =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = static_cast<VkBool32>(useDepth ? VK_TRUE : VK_FALSE),
        .depthWriteEnable = static_cast<VkBool32>(useDepth ? VK_TRUE : VK_FALSE),
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f
    };

    const VkGraphicsPipelineCreateInfo pipelineInfo =
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pTessellationState = nullptr,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = useDepth ? &depthStencil : nullptr,
        .pColorBlendState = &colorBlending,
        .pDynamicState = nullptr,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    VK_CHECK(vkCreateGraphicsPipelines(renderDevice.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, graphicsPipeline));

    for (auto m : shaderModules)
        vkDestroyShaderModule(renderDevice.device, m.shaderModule, nullptr);

    return true;
}

bool CreateGraphicsPipeline(
    const ms::VulkanRenderDevice& renderDevice, ms::VulkanState& state,
    const std::vector<std::string>& shaderFiles,
    VkPrimitiveTopology topology,
    bool useDepth, bool useBlending, bool dynamicScissorState,
    int32_t customWidth, int32_t customHeight,
    uint32_t numPatchControlPoints
)
{
    return CreateGraphicsPipeline(renderDevice, state.renderPass, state.pipelineLayout, &state.graphicsPipeline, shaderFiles, topology, useDepth, useBlending, dynamicScissorState, customWidth, customHeight, numPatchControlPoints);
}

bool CreateColorAndDepthFramebuffers(const ms::VulkanRenderDevice& renderDevice, ms::VulkanState& state)
{
    return CreateColorAndDepthFramebuffers(renderDevice, state.renderPass, state.depthTexture.imageView, state.swapchainFramebuffers);
}

bool CreateColorAndDepthFramebuffers(const ms::VulkanRenderDevice& renderDevice, VkRenderPass renderPass, VkImageView& imageView, std::vector<VkFramebuffer>& swapchainFramebuffers)
{
    swapchainFramebuffers.resize(renderDevice.swapchainImageViews.size());

    for (size_t i = 0; i < renderDevice.swapchainImages.size(); i++) {
        std::vector<VkImageView> attachments = {
            renderDevice.swapchainImageViews[i],
            imageView
        };

        const VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = renderPass,
            .attachmentCount = static_cast<uint32_t>((imageView == VK_NULL_HANDLE) ? 1 : 2),
            .pAttachments = attachments.data(),
            .width = renderDevice.framebufferWidth,
            .height = renderDevice.framebufferHeight,
            .layers = 1
        };

        VK_CHECK(vkCreateFramebuffer(renderDevice.device, &framebufferInfo, nullptr, &swapchainFramebuffers[i]));
    }

    return true;
}

#pragma region 6 Vulkan Pipeline

bool _CreateShaderModule(VkDevice device, ms::ShaderModule* shaderModule, std::string fileName, glslang_stage_t stage)
{
    std::string tempStr;
    if (!_CompileShaderFile(fileName.c_str(), stage, shaderModule))
    {
        throw std::runtime_error("GLSL compile shader failed.\n");
    }
    tempStr = fileName + ".spv";
    _SaveSPIRVBinaryFile(tempStr.c_str(), shaderModule->SPIRV.data(), shaderModule->SPIRV.size());

    const VkShaderModuleCreateInfo createInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shaderModule->SPIRV.size() * sizeof(unsigned int),
        .pCode = shaderModule->SPIRV.data()
    };

    VK_CHECK_RET(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule->shaderModule));
}

size_t _CompileShaderFile(const char* file, glslang_stage_t stage, ms::ShaderModule* shaderModule)
{
    if (auto shaderSource = readShaderFile(file); !shaderSource.empty())
    {
        return _CompileShader(stage, shaderSource.c_str(), shaderModule);
    }

    return 0;
}

size_t _CompileShader(glslang_stage_t stage, const char* shaderSource, ms::ShaderModule* shaderModule)
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
    shaderModule->SPIRV.resize(glslang_program_SPIRV_get_size(program));
    glslang_program_SPIRV_get(program, shaderModule->SPIRV.data());
    const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
    if (spirv_messages)
    {
        std::cout << spirv_messages << std::endl;
    }

    glslang_program_delete(program);
    glslang_shader_delete(shader);

    return shaderModule->SPIRV.size();
}

void _SaveSPIRVBinaryFile(const char* filename, unsigned int* code, size_t size)
{
    FILE* f = fopen(filename, "w");

    if (!f)
        return;

    fwrite(code, sizeof(uint32_t), size, f);
    fclose(f);
}

#pragma endregion

void UploadBufferData(uint32_t currentImage, ms::VulkanState& state, ms::VulkanRenderDevice& rendererDevice, const ms::UniformBuffer& ubo)
{
    void* data = nullptr;
    vkMapMemory(rendererDevice.device, state.uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(rendererDevice.device, state.uniformBuffersMemory[currentImage]);
}

void UploadBufferData(const ms::VulkanRenderDevice& rendererDevice, const VkDeviceMemory& bufferMemory, VkDeviceSize deviceOffset, const void* data, const size_t dataSize)
{
    EASY_FUNCTION();

    void* mapperData = nullptr;
    vkMapMemory(rendererDevice.device, bufferMemory, deviceOffset, dataSize, 0, &mapperData);
    memcpy(mapperData, data, dataSize);
    vkUnmapMemory(rendererDevice.device, bufferMemory);
}

bool FillCommandBuffers(ms::VulkanRenderDevice& renderDevice, ms::VulkanState& state, const unsigned int indexBufferSize, const unsigned int width, const unsigned int height, size_t i)
{
    const VkCommandBufferBeginInfo commandBufferBeginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        .pInheritanceInfo = nullptr
    };

    const std::vector<VkClearValue> clearValues =
    {
        VkClearValue { .color = { 0.5f, 0.2f, 0.7f, 1.f } },
        VkClearValue { .depthStencil = {.depth = 1.0f, .stencil = 0 } }
    };

    const VkRect2D screenRect =
    {
        .offset = { 0, 0 },
        .extent = {.width = width, .height = height }
    };

    VK_CHECK(vkBeginCommandBuffer(renderDevice.commandBuffers[i], &commandBufferBeginInfo));

    const VkRenderPassBeginInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = state.renderPass,
        .framebuffer = state.swapchainFramebuffers[i],
        .renderArea = screenRect,
        .clearValueCount = static_cast<uint32_t>(clearValues.size()),
        .pClearValues = clearValues.data()
    };

    vkCmdBeginRenderPass(renderDevice.commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(renderDevice.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, state.graphicsPipeline);
    vkCmdBindDescriptorSets(renderDevice.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipelineLayout, 0, 1, &state.descriptorSets[i], 0, nullptr);
    vkCmdDraw(renderDevice.commandBuffers[i], static_cast<uint32_t>(indexBufferSize / sizeof(uint32_t)), 1, 0, 0);
    vkCmdEndRenderPass(renderDevice.commandBuffers[i]);

    VK_CHECK_RET(vkEndCommandBuffer(renderDevice.commandBuffers[i]));
}

// TODO

VkFormat findSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    const bool isLinear  = tiling == VK_IMAGE_TILING_LINEAR;
    const bool isOptimal = tiling == VK_IMAGE_TILING_OPTIMAL;

    for (VkFormat format : candidates)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device, format, &properties);

        if (isLinear && (properties.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (isOptimal && (properties.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    ASSERT(false);
    return VK_FORMAT_UNDEFINED;
}

bool isDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    const bool isDiscreteGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    const bool isIntegratedGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    const bool isGPU = isDiscreteGPU || isIntegratedGPU;

    return isGPU && deviceFeatures.geometryShader;
}
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

#pragma region Stuctures

namespace ms
{
    enum eRenderPassBit : uint8_t
    {
        // clear the attachment
        eRenderPassBit_First = 0x01,
        // transition to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        eRenderPassBit_Last = 0x02,
        // transition to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        eRenderPassBit_Offscreen = 0x04,
        // keep VK_IMAGE_LAYOUT_*_ATTACHMENT_OPTIMAL
        eRenderPassBit_OffscreenInternal = 0x08,
    };

    struct RenderPassCreateInfo final
    {
        bool clearColor_ = false;
        bool clearDepth_ = false;
        uint8_t flags_ = 0;
    };

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

    struct VulkanTexture
    {
        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;
    };

    struct UniformBuffer
    {
        glm::mat4 mp;
    };

    struct VulkanImage final
    {
        VkImage image = nullptr;
        VkDeviceMemory imageMemory = nullptr;
        VkImageView imageView = nullptr;
    };

    struct VulkanInstance
    {
        // VkInstance - Represents the connection from your application to the Vulkan runtime.
        VkInstance instance;
        // VkSurfaceKHR - Represents an abstract type of surface to present rendered images to.
        VkSurfaceKHR surface;
        VkDebugUtilsMessengerEXT messenger;
        VkDebugReportCallbackEXT reportCallback;
    };

    struct VulkanRenderDevice
    {
        // VkPhysicalDevice - Represents a specific Vulkan-compatible device, like a graphics card.
        VkPhysicalDevice physicalDevice;
        // VkDevice - Represents an initialized Vulkan device that is ready to create all other objects.
        VkDevice device;
        // VkSwapchainKHR - Represents a set of images that can be presented on the Surface.
        // Swapchain is a concept used to present the final image on the screen or inside the window you're drawing into.
        VkSwapchainKHR swapchain;
        // VkQueue - Represents a queue of commands to be executed on the device.
        VkQueue graphicsQueue;

        // VkCommandPool - Command pools are opaque objects that Command buffers Memory is allocated from.
        VkCommandPool commandPool;
        // VkCommandBuffer - Represents a buffer of various commands to be executed by a logical device.
        std::vector<VkCommandBuffer> commandBuffers;

        // VkImage - Represent a set of pixels.
        // Image is a type of resource that occupy device memory.
        std::vector<VkImage> swapchainImages;
        // VkImageView - Image objects are not directly accessed by pipeline shaders for reading or writing image data.
        // Instead, image views representing contiguous ranges of the image subresources and containing additional metadata are used for that purpose.
        std::vector<VkImageView> swapchainImageViews;

        VkSemaphore semaphore;
        VkSemaphore renderSemaphore;

        mutable size_t indexBufferSize;
        uint32_t framebufferWidth;
        uint32_t framebufferHeight;
        uint32_t graphicsFamily;
    };

    struct VulkanState
    {        
        // VkDescriptorSetLayout - Before creating a Descriptor Set, its layout must be specified by creating a DescriptorSetLayout.
        VkDescriptorSetLayout descriptorSetLayout;
        // VkDescriptorPool - Object used to allocate Descriptor Sets.
        VkDescriptorPool descriptorPool;
        // VkDescriptorSet - The way shaders can access resources (Buffers, Images and Samplers) is through Descriptors.
        // Descriptors don't exist on their own, but are grouped in Descriptor Sets.
        std::vector<VkDescriptorSet> descriptorSets;

        // VkFramebuffer - Represents a link to actual Images that can be use as attachments.
        // Attachment is the Vulkan's name for Render Target. An Image to be use as output from rendering.
        std::vector<VkFramebuffer> swapchainFramebuffers;

        // VkPipelineLayout - Represents a configuration of the rendering pipeline in terms of what types of Descriptor Sets will be bound to the Command Buffer.
        VkPipelineLayout pipelineLayout;
        // VkPipeline - Is a big object, as it composes most of the objects. It represents the configuration of the whole pipeline and has a lot of parameters.
        VkPipeline graphicsPipeline;
        // VkRenderPass - In Vulkan you need to plan the rendering of your frame in advance and organize it into Passes and Subpasses.
        VkRenderPass renderPass;

        // 4. Uniform buffer
        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;

        // 5. Depth buffer
        VulkanImage depthTexture;

        // 6. Duck Model
        VkBuffer storageBuffer;
        VkDeviceMemory storageBufferMemory;

        // 7. Duck Model Texture
        VkSampler textureSampler;
        VulkanImage texture;
    };
}

#pragma endregion

void CreateVulkanInstance(VkInstance* instance);

#pragma region 1 Vulkan Instance

bool SetupDebugCallbaks(VkInstance* instance, VkDebugUtilsMessengerEXT* messenger, VkDebugReportCallbackEXT* reportCallback);

static VKAPI_ATTR VkBool32 VKAPI_CALL _VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT secerity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* calbackData, void* userData);

static VKAPI_ATTR VkBool32 VKAPI_CALL _VulkanDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* userData);

#pragma endregion

bool CreateVulkanRenderDevice(ms::VulkanInstance* instance, ms::VulkanRenderDevice* renderDevice, uint32_t width, uint32_t height, std::function<bool(VkPhysicalDevice)> selector, VkPhysicalDeviceFeatures deviceFeatures);

#pragma region 2 Vulkan Render Device

//      2.1 VkDevice  
bool _FindSuitablePhysicalDevice(VkInstance instance, std::function<bool(VkPhysicalDevice)> selector, VkPhysicalDevice* physicalDevice);

uint32_t _FindQueueFamilies(VkPhysicalDevice physicalDevice, VkQueueFlags desiredFlags);

bool _CreateDevice(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures deviceFeatures, uint32_t graphicsFamily, VkDevice* device);

//      2.2 VkSwapchain
bool _CreateSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t graphicsFamily, uint32_t width, uint32_t height, VkSwapchainKHR* swapchain, bool supportScreenshots);

ms::SwapChainSupportDetails _QuerySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

VkSurfaceFormatKHR _ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);

VkPresentModeKHR _ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

uint32_t _ChooseSwapImageCount(const VkSurfaceCapabilitiesKHR& capabilities);

//      2.3 VkImages and VkImageViews
size_t _CreateSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage>& swapchainImages, std::vector<VkImageView>& swapchainImageViews);

bool CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, uint32_t layerCount = 1, uint32_t mipLevels = 1);

//      2.4 VkSemaphore
bool _CreateSemaphore(VkDevice device, VkSemaphore* outSemaphore);

#pragma endregion

bool CreateTexturedVertexBuffer(const ms::VulkanRenderDevice& renderDevice, const char* fileName, VkBuffer* storageBuffer, VkDeviceMemory* storageBufferMemory, size_t* vertexBufferSize, size_t* indexBufferSize);

#pragma region 3 Textured Vertex Buffer

// Create the GPU Buffer
bool CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

// Create a Uniform Buffer
bool CreateUniformBuffers(ms::VulkanState& vkState, ms::VulkanRenderDevice& rendererDevice);

uint32_t _FindMemoryType(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

// Copy data into the GPU Buffer
void _CopyBuffer(const ms::VulkanRenderDevice & renderDevice, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

#pragma endregion

bool CreateTextureImage(const ms::VulkanRenderDevice & device, const char* fileName, VkImage& textureImage, VkDeviceMemory& textureImageMemory);

bool CreateTextureSampler(VkDevice device, VkSampler* sampler);

#pragma region 4 Texture Image and Texture Sampler

bool _CreateImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling VkImageTiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

void _TransitionImageLayout(const ms::VulkanRenderDevice & renderDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount = 1, uint32_t mipLevels = 1);

void _TransitionImageLayoutCmd(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount, uint32_t mipLevels);

bool _HasStencilComponent(VkFormat format);

void _CopyBufferToImage(ms::VulkanRenderDevice device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

#pragma endregion

bool CreateDepthResources(const ms::VulkanRenderDevice & renderDevice, uint32_t width, uint32_t height, ms::VulkanImage& depth);

#pragma region 5 Depth Resources

VkFormat _FindDepthFormat(VkPhysicalDevice device);

#pragma endregion

bool CreateDescriptorPool(VkDevice device, uint32_t imageCount, uint32_t uniformBufferCount, uint32_t storageBufferCount, uint32_t samplerCount, VkDescriptorPool* descPool);

bool CreateDescriptorSet(ms::VulkanRenderDevice& renderDevice, ms::VulkanState& state, uint32_t vertexBufferSize, uint32_t indexBufferSize);

bool CreateColorAndDepthRenderPass(const ms::VulkanRenderDevice& renderDevice, bool useDepth, VkRenderPass* renderPass, const ms::RenderPassCreateInfo& ci, VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM);

bool CreatePipelineLayout(const ms::VulkanRenderDevice& renderDevice, VkDescriptorSetLayout dsLayout, VkPipelineLayout* pipelineLayout);

bool CreateGraphicsPipeline(
    const ms::VulkanRenderDevice& renderDevice,
    VkRenderPass renderPass, VkPipelineLayout pipelineLyout, VkPipeline* graphicsPipeline,
    const std::vector<std::string>& shaderFiles,
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST /* defaults to triangles*/,
    bool useDepth = true, bool useBlending = true, bool dynamicScissorState = false,
    int32_t customWidth = -1, int32_t customHeight = -1,
    uint32_t numPatchControlPoints = 0, bool hasGeom = true
);

bool CreateGraphicsPipeline(
    const ms::VulkanRenderDevice& renderDevice, ms::VulkanState& state,
    const std::vector<std::string>& shaderFiles,
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST /* defaults to triangles*/,
    bool useDepth = true, bool useBlending = true, bool dynamicScissorState = false,
    int32_t customWidth = -1, int32_t customHeight = -1,
    uint32_t numPatchControlPoints = 0
);

bool CreateColorAndDepthFramebuffers(const ms::VulkanRenderDevice& renderDevice, ms::VulkanState& state);

bool CreateColorAndDepthFramebuffers(const ms::VulkanRenderDevice& renderDevice, VkRenderPass renderPass, VkImageView& imageView, std::vector<VkFramebuffer>& swapchainFramebuffers);

#pragma region 6 Vulkan Pipeline

bool _CreateShaderModule(VkDevice device, ms::ShaderModule* sm, std::string fileName, glslang_stage_t stage);

size_t _CompileShader(glslang_stage_t stage, const char* shaderSource, ms::ShaderModule* shaderModule);

size_t _CompileShaderFile(const char* file, glslang_stage_t stage, ms::ShaderModule* shaderModule);

void _SaveSPIRVBinaryFile(const char* filename, unsigned int* code, size_t size);

#pragma endregion

void UploadBufferData(uint32_t currentImage, ms::VulkanState& vkState, ms::VulkanRenderDevice& rendererDevice, const ms::UniformBuffer& ubo);

void UploadBufferData(const ms::VulkanRenderDevice& rendererDevice, const VkDeviceMemory& bufferMemory, VkDeviceSize deviceOffset, const void* data, const size_t dataSize);

bool FillCommandBuffers(ms::VulkanRenderDevice& renderDevice, ms::VulkanState& state, const unsigned int indexBufferSize, const unsigned int width, const unsigned int height, size_t i);

// TODO

VkFormat findSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

bool isDeviceSuitable(VkPhysicalDevice device);
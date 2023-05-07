#include "mspch.h"

#include "msAppVulkan.h"

#include "platform/vulkan/utils/UtilsVulkan.h"
#include "glm/gtx/transform.hpp"

namespace ms
{
    MsAppVulkan::MsAppVulkan()
    {
        Initialize();
    }

    MsAppVulkan::MsAppVulkan(unsigned int w, unsigned int h, std::string name)
        : mWindow { w, h, name }
    {
        Initialize();
    }

    MsAppVulkan::~MsAppVulkan()
    {
        Destroy();
    }

    void MsAppVulkan::Initialize()
    {
        glslang_initialize_process();

        CreateVulkanInstance(&mVulkanInstance.instance);

        if (false)
        {
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            std::cout << "Vulkan has " << extensionCount << " extensions supported\n";
        }

        if (!SetupDebugCallbaks(&mVulkanInstance.instance, &mVulkanInstance.messenger, &mVulkanInstance.reportCallback))
        {
            exit(EXIT_FAILURE);
        }

        VK_CHECK(glfwCreateWindowSurface(mVulkanInstance.instance, mWindow.GetGLFWWindow(), nullptr, &mVulkanInstance.surface));

        if (!CreateVulkanRenderDevice(&mVulkanInstance, &mVulkanRenderDevice, mWindow.GetWidth(), mWindow.GetHeight(), isDeviceSuitable, { .geometryShader = VK_TRUE }))
        {
            exit(EXIT_FAILURE);
        }

        size_t vertexBufferSize;
        size_t indexBufferSize;

        if(
            !CreateTexturedVertexBuffer(mVulkanRenderDevice, "rsc/models/bunny/bunny.obj", &mVulkanState.storageBuffer,
                &mVulkanState.storageBufferMemory, &vertexBufferSize, &indexBufferSize)
            || !CreateUniformBuffers(mVulkanState, mVulkanRenderDevice)
        )
        {
            exit(EXIT_FAILURE);
        }

        if(
            !CreateTextureImage(mVulkanRenderDevice, "rsc/textures/example/example.png", mVulkanState.texture.image, mVulkanState.texture.imageMemory)
            || !CreateImageView(mVulkanRenderDevice.device, mVulkanState.texture.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &mVulkanState.texture.imageView)
            || ! CreateTextureSampler(mVulkanRenderDevice.device, &mVulkanState.textureSampler)
        )
        {
            exit(EXIT_FAILURE);
        }

        CreateDepthResources(mVulkanRenderDevice, mWindow.GetWidth(), mWindow.GetHeight(), mVulkanState.depthTexture);

        CreateDescriptorPool(mVulkanRenderDevice.device, mVulkanRenderDevice.swapchainImages.size(), 1, 2, 1, &mVulkanState.descriptorPool);

        CreateDescriptorSet(mVulkanRenderDevice, mVulkanState, vertexBufferSize, indexBufferSize);

        const RenderPassCreateInfo renderPassCreateInfo =
        {
            .clearColor_ = true,
            .clearDepth_ = true,
            .flags_ = eRenderPassBit_First | eRenderPassBit_Last
        };
        CreateColorAndDepthRenderPass(mVulkanRenderDevice, true, &mVulkanState.renderPass, renderPassCreateInfo, VK_FORMAT_R8G8B8A8_UNORM);

        CreatePipelineLayout(mVulkanRenderDevice, mVulkanState.descriptorSetLayout, &mVulkanState.pipelineLayout);

        const std::vector<std::string>& shaderFiles =
        {
            "rsc/shaders/vkPVPDefault.vert",
            "rsc/shaders/vkPVPDefault.frag",
            "rsc/shaders/vkPVPDefault.geom"
        };
        CreateGraphicsPipeline(mVulkanRenderDevice, mVulkanState, shaderFiles);

        CreateColorAndDepthFramebuffers(mVulkanRenderDevice, mVulkanState);

        glslang_finalize_process();
    }

    bool MsAppVulkan::DrawOverlay()
    {
        uint32_t imageIndex = 0;

        if (vkAcquireNextImageKHR(mVulkanRenderDevice.device, mVulkanRenderDevice.swapchain, 0, mVulkanRenderDevice.semaphore, VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS)
            return false;

        VK_CHECK(vkResetCommandPool(mVulkanRenderDevice.device, mVulkanRenderDevice.commandPool, 0));

        int width, height;

        glfwGetFramebufferSize(mWindow.GetGLFWWindow(), &width, &height);

        const float ratio = width / (float)height;

        const glm::mat4 m1 =
            glm::rotate(
                glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.75f, -2.f))
                *
                glm::rotate(glm::mat4(1.f), glm::pi<float>() * 1.f, glm::vec3(1.f, 0.f, 0.f))
              , (float)glfwGetTime()
              , glm::vec3(0.0f, 1.0f, 0.0f)
            );

        const glm::mat4 p = glm::perspective(45.f, ratio, .1f, 1000.f);

        const UniformBuffer ubo{ .mp = p * m1 };
        UploadBufferData(imageIndex, mVulkanState, mVulkanRenderDevice, ubo);

        FillCommandBuffers(mVulkanRenderDevice, mVulkanState, mVulkanRenderDevice.indexBufferSize, width, height, imageIndex);

        const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        const VkSubmitInfo si =
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &mVulkanRenderDevice.semaphore,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &mVulkanRenderDevice.commandBuffers[imageIndex],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &mVulkanRenderDevice.renderSemaphore
        };

        VK_CHECK(vkQueueSubmit(mVulkanRenderDevice.graphicsQueue, 1, &si, nullptr));

        const VkPresentInfoKHR pi =
        {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &mVulkanRenderDevice.renderSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &mVulkanRenderDevice.swapchain,
            .pImageIndices = &imageIndex
        };
        VK_CHECK(vkQueuePresentKHR(mVulkanRenderDevice.graphicsQueue, &pi));
        VK_CHECK(vkDeviceWaitIdle(mVulkanRenderDevice.device));

        return true;
    }

    void MsAppVulkan::Destroy()
    {
        // Release Duck Model
        vkDestroyBuffer(mVulkanRenderDevice.device, mVulkanState.storageBuffer, nullptr);
        vkFreeMemory(mVulkanRenderDevice.device, mVulkanState.storageBufferMemory, nullptr);

        // Release Duck Texture
        vkDestroySampler(mVulkanRenderDevice.device, mVulkanState.textureSampler, nullptr);
        vkDestroyImageView(mVulkanRenderDevice.device, mVulkanState.texture.imageView, nullptr);
        vkDestroyImage(mVulkanRenderDevice.device, mVulkanState.texture.image, nullptr);
        vkFreeMemory(mVulkanRenderDevice.device, mVulkanState.texture.imageMemory, nullptr);

        // Free Uniform Buffer
        for (size_t i = 0; i < mVulkanState.uniformBuffers.size(); i++)
        {
            vkDestroyBuffer(mVulkanRenderDevice.device, mVulkanState.uniformBuffers[i], nullptr);
            vkFreeMemory(mVulkanRenderDevice.device, mVulkanState.uniformBuffersMemory[i], nullptr);
        }

        // Free Descriptor Set related objects
        vkDestroyDescriptorSetLayout(mVulkanRenderDevice.device, mVulkanState.descriptorSetLayout, nullptr);
        vkDestroyDescriptorPool(mVulkanRenderDevice.device, mVulkanState.descriptorPool, nullptr);

        // Free Frame Buffers
        for (VkFramebuffer frameBuffer : mVulkanState.swapchainFramebuffers)
        {
            vkDestroyFramebuffer(mVulkanRenderDevice.device, frameBuffer, nullptr);
        }

        // Free Depth Texture
        vkDestroyImageView(mVulkanRenderDevice.device, mVulkanState.depthTexture.imageView, nullptr);
        vkDestroyImage(mVulkanRenderDevice.device, mVulkanState.depthTexture.image, nullptr);
        vkFreeMemory(mVulkanRenderDevice.device, mVulkanState.depthTexture.imageMemory, nullptr);

        // Free Render Pass
        vkDestroyRenderPass(mVulkanRenderDevice.device, mVulkanState.renderPass, nullptr);

        // Free Pipeline related objects
        vkDestroyPipelineLayout(mVulkanRenderDevice.device, mVulkanState.pipelineLayout, nullptr);
        vkDestroyPipeline(mVulkanRenderDevice.device, mVulkanState.graphicsPipeline, nullptr);

        // VulkanRenderDevice
        vkDestroySemaphore(mVulkanRenderDevice.device, mVulkanRenderDevice.semaphore, nullptr);
        vkDestroySemaphore(mVulkanRenderDevice.device, mVulkanRenderDevice.renderSemaphore, nullptr);

        vkDestroySwapchainKHR(mVulkanRenderDevice.device, mVulkanRenderDevice.swapchain, nullptr);

        for (VkImageView imageView : mVulkanRenderDevice.swapchainImageViews)
        {
            vkDestroyImageView(mVulkanRenderDevice.device, imageView, nullptr);
        }

        vkDestroyCommandPool(mVulkanRenderDevice.device, mVulkanRenderDevice.commandPool, nullptr);

        vkDestroyDevice(mVulkanRenderDevice.device, nullptr);

        // VulkanInstance
        vkDestroySurfaceKHR(mVulkanInstance.instance, mVulkanInstance.surface, nullptr);

        vkDestroyDebugReportCallbackEXT(mVulkanInstance.instance, mVulkanInstance.reportCallback, nullptr);
        vkDestroyDebugUtilsMessengerEXT(mVulkanInstance.instance, mVulkanInstance.messenger, nullptr);

        vkDestroyInstance(mVulkanInstance.instance, nullptr);
    }

    void MsAppVulkan::Run()
    {
        EASY_PROFILER_ENABLE;
        EASY_MAIN_THREAD;

        OPTICK_THREAD("Vulkan App Thread");
        OPTICK_START_CAPTURE()

        GLFWwindow* window = mWindow.GetGLFWWindow();

        while (!glfwWindowShouldClose(window))
        {
            EASY_BLOCK("Vulkan App Main loop");

            OPTICK_PUSH("Vulkan App Main Loop");

            DrawOverlay();
            glfwPollEvents();

            OPTICK_POP();
        }

        OPTICK_STOP_CAPTURE();
        OPTICK_SAVE_CAPTURE("VulkanApp_Optick");

        profiler::dumpBlocksToFile("VulkanApp_Easy");
        EASY_PROFILER_DISABLE;
    }
}

#include "mspch.h"

#include "msAppVulkan.h"

#include "platform/vulkan/utils/UtilsVulkan.h"
#include "platform/vulkan/vkVulkanClear.h"
#include "platform/vulkan/vkVulkanFinish.h"
#include "platform/vulkan/vkVulkanModelRenderer.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

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
        EASY_PROFILER_ENABLE;
        EASY_MAIN_THREAD;

        OPTICK_THREAD("Vulkan App Thread");
        OPTICK_START_CAPTURE();

        EASY_BLOCK("Initialize");
        OPTICK_PUSH("Initialize");

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

        EASY_END_BLOCK;
        OPTICK_STOP_CAPTURE();
    }

    void MsAppVulkan::Destroy()
    {
        // VulkanRenderDevice
        for (VkImageView imageView : mVulkanRenderDevice.swapchainImageViews)
        {
            vkDestroyImageView(mVulkanRenderDevice.device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(mVulkanRenderDevice.device, mVulkanRenderDevice.swapchain, nullptr);

        vkDestroyCommandPool(mVulkanRenderDevice.device, mVulkanRenderDevice.commandPool, nullptr);

        vkDestroySemaphore(mVulkanRenderDevice.device, mVulkanRenderDevice.semaphore, nullptr);
        vkDestroySemaphore(mVulkanRenderDevice.device, mVulkanRenderDevice.renderSemaphore, nullptr);

        vkDestroyDevice(mVulkanRenderDevice.device, nullptr);

        // VulkanInstance
        vkDestroySurfaceKHR(mVulkanInstance.instance, mVulkanInstance.surface, nullptr);

        vkDestroyDebugReportCallbackEXT(mVulkanInstance.instance, mVulkanInstance.reportCallback, nullptr);
        vkDestroyDebugUtilsMessengerEXT(mVulkanInstance.instance, mVulkanInstance.messenger, nullptr);

        vkDestroyInstance(mVulkanInstance.instance, nullptr);

        OPTICK_STOP_CAPTURE();
        OPTICK_SAVE_CAPTURE("VulkanApp_Optick");

        profiler::dumpBlocksToFile("VulkanApp_Easy");
        EASY_PROFILER_DISABLE;
    }

    void MsAppVulkan::Run()
    {
        GLFWwindow* window = mWindow.GetGLFWWindow();

        std::unique_ptr<vkVulkanModelRenderer> model;
        std::unique_ptr<vkVulkanClear> clear;
        std::unique_ptr<vkVulkanFinish> finish;

        {
            EASY_BLOCK("Create Renderers");
            OPTICK_PUSH("Create Renderers");

            glslang_initialize_process();

            model = std::make_unique<vkVulkanModelRenderer>(mVulkanRenderDevice, "./rsc/models/bunny/bunny.obj", "./rsc/textures/example/example.png", (uint32_t)sizeof(UniformBuffer));
            clear = std::make_unique<vkVulkanClear>(mVulkanRenderDevice, model->GetDepthTexture());
            finish = std::make_unique<vkVulkanFinish>(mVulkanRenderDevice, model->GetDepthTexture());

            glslang_finalize_process();

            EASY_END_BLOCK;
            OPTICK_POP();
        }

        while (!glfwWindowShouldClose(window))
        {
            EASY_BLOCK("Main loop");
            OPTICK_PUSH("Main Loop");

            uint32_t imageIndex = 0;
            VkResult result = vkAcquireNextImageKHR(mVulkanRenderDevice.device, mVulkanRenderDevice.swapchain, 0, mVulkanRenderDevice.semaphore, VK_NULL_HANDLE, &imageIndex);
            VK_CHECK(vkResetCommandPool(mVulkanRenderDevice.device, mVulkanRenderDevice.commandPool, 0));

            if (result != VK_SUCCESS) continue;

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
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

            model.get()->UpdateUniformBuffer(mVulkanRenderDevice, imageIndex, &ubo, sizeof(UniformBuffer));

            VkCommandBuffer commandBuffer = mVulkanRenderDevice.commandBuffers[imageIndex];

            const VkCommandBufferBeginInfo bi =
            {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                .pInheritanceInfo = nullptr
            };

            VK_CHECK(vkBeginCommandBuffer(commandBuffer, &bi));

            clear.get()->FillCommandBuffer(commandBuffer, imageIndex);
            model.get()->FillCommandBuffer(commandBuffer, imageIndex);
            finish.get()->FillCommandBuffer(commandBuffer, imageIndex);

            VK_CHECK(vkEndCommandBuffer(commandBuffer));

            const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // or even VERTEX_SHADER_STAGE

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

            {
                EASY_BLOCK("vkQueueSubmit", profiler::colors::Magenta);
                VK_CHECK(vkQueueSubmit(mVulkanRenderDevice.graphicsQueue, 1, &si, nullptr));
                EASY_END_BLOCK;
            }

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

            {
                EASY_BLOCK("vkQueuePresentKHR", profiler::colors::Magenta);
                VK_CHECK(vkQueuePresentKHR(mVulkanRenderDevice.graphicsQueue, &pi));
                EASY_END_BLOCK;
            }

            {
                EASY_BLOCK("vkDeviceWaitIdle", profiler::colors::Red);
                VK_CHECK(vkDeviceWaitIdle(mVulkanRenderDevice.device));
                EASY_END_BLOCK;
            }

            glfwPollEvents();

            EASY_END_BLOCK;
            OPTICK_POP();
        }
    }
}

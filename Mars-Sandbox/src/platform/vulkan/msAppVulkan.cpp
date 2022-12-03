#include "mspch.h"

#include "msAppVulkan.h"

#include "platform/vulkan/utils/UtilsVulkan.h"

namespace ms
{
    MsAppVulkan::MsAppVulkan()
    {
        Initialize();
    }

    void MsAppVulkan::Initialize()
    {
        createInstance(mVkInstance);
    }

    void MsAppVulkan::Run()
    {
        EASY_PROFILER_ENABLE;
        EASY_MAIN_THREAD;

        OPTICK_THREAD("Vulkan App Thread");
        OPTICK_START_CAPTURE()

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::cout << "Vulkan has " << extensionCount << " extensions supported\n";

        glslang_initialize_process();
        ShaderModule shaderModule;
        if (compileShaderFile("rsc/shaders/default.vert", GLSLANG_STAGE_VERTEX, shaderModule) < 0)
        {
            throw std::runtime_error("GLSL compile VERTEX sahder failed\n");
        }
        saveSPIRVBinaryFile("rsc/shaders/default.vert.spv", shaderModule.SPIRV.data(), shaderModule.SPIRV.size());

        if (compileShaderFile("rsc/shaders/default.frag", GLSLANG_STAGE_FRAGMENT, shaderModule) < 0)
        {
            throw std::runtime_error("GLSL compile FRAGMENT sahder failed\n");
        }
        saveSPIRVBinaryFile("rsc/shaders/default.frag.spv", shaderModule.SPIRV.data(), shaderModule.SPIRV.size());
        glslang_finalize_process();

        GLFWwindow* window = mWindow.GetGLFWWindow();

        while (!glfwWindowShouldClose(window))
        {
            EASY_BLOCK("Vulkan App Main loop");

            OPTICK_PUSH("Vulkan App Main Loop");

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            glfwPollEvents();

            OPTICK_POP();
        }

        OPTICK_STOP_CAPTURE();
        OPTICK_SAVE_CAPTURE("VulkanApp_Optick");

        profiler::dumpBlocksToFile("VulkanApp_Easy");
        EASY_PROFILER_DISABLE;
    }
}

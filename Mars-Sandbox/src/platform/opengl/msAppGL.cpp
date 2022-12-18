#include "mspch.h"

#include "msAppGL.h"
#include "msShaderGL.h"

#include "stb/stb_image_write.h"

namespace ms
{
    MsAppGL::MsAppGL()
    {
        Initialize();
    }

    MsAppGL::MsAppGL(unsigned int w, unsigned int h, std::string name)
        : mWindow{ w, h, name }
    {
        Initialize();
    }

    void MsAppGL::Initialize()
    {
        mWindow.SetGLFWKeyCallback(
            [](GLFWwindow* window, int key, int scancode, int action, int mods)
            {
                if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                {
                    glfwSetWindowShouldClose(window, true);
                }
                if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
                {
                    int i_width, i_height;
                    glfwGetFramebufferSize(window, &i_width, &i_height);
                    uint8_t* ptr = (uint8_t*)malloc(i_width * i_height * 4);
                    glReadPixels(0, 0, i_width, i_height, GL_RGBA, GL_UNSIGNED_BYTE, ptr);
                    stbi_write_png("screenshot.png", i_width, i_height, 4, ptr, 0);
                    free(ptr);
                }
            }
        );
    }

    void MsAppGL::Run()
    {
        EASY_PROFILER_ENABLE;
        EASY_MAIN_THREAD;

        OPTICK_THREAD("Open GL App Thread");
        OPTICK_START_CAPTURE()

        GLint minor, major;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        std::cout << "Open GL Version " << major << "." << minor;

        GLFWwindow* window = mWindow.GetGLFWWindow();

        while (!glfwWindowShouldClose(window))
        {
            EASY_BLOCK("Open GL App Main loop");

            OPTICK_PUSH("Open GL App Main Loop");

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            ImGui::ShowDemoWindow();

            // Rendering
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows
            // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
            //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            glfwPollEvents();

            OPTICK_POP();
        }

        OPTICK_STOP_CAPTURE();
        OPTICK_SAVE_CAPTURE("OpenGLApp");

        profiler::dumpBlocksToFile("OpenGLApp_Easy");
        EASY_PROFILER_DISABLE;
    }
}
#include "mspch.h"

#include "msWindowVulkan.h"

namespace ms
{
	MsWindowVulkan::~MsWindowVulkan()
	{
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	void MsWindowVulkan::SetGLFWKeyCallback(GLFWkeyfun function)
	{
		glfwSetKeyCallback(mWindow, function);
	}

	void MsWindowVulkan::InitializeWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		mWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);

		SetGLFWKeyCallback(
			[](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
					glfwSetWindowShouldClose(window, GLFW_TRUE);
			}
		);
	}

	void MsWindowVulkan::InitializImGui()
	{
		
	}
}
#include "mspch.h"

#include "msWindowVulkan.h"

namespace ms
{
	MsWindowVulkan::~MsWindowVulkan()
	{
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	void MsWindowVulkan::InitializeWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		mWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);
	}

	void MsWindowVulkan::InitializImGui()
	{

	}
}
#pragma once

#include "platform/common/msWindow.h"

namespace ms
{
	class MsWindowVulkan : public MsWindow
	{
	public:
		MsWindowVulkan() : MsWindow()
		{
			InitializeWindow();
			InitializImGui();
		};

		MsWindowVulkan(unsigned int w, unsigned int h, std::string name)
			: MsWindow(w, h, name)
		{
			InitializeWindow();
			InitializImGui();
		};
		virtual ~MsWindowVulkan();

		// Remove Copy Constructor and Copy Assignment Operator
		MsWindowVulkan(const MsWindowVulkan&) = delete;
		MsWindowVulkan& operator=(const MsWindowVulkan&) = delete;
		// Remove Move Constructor and Move Assignment Operator
		MsWindowVulkan(MsWindowVulkan&&) = delete;
		MsWindowVulkan& operator=(MsWindowVulkan&&) = delete;

		inline GLFWwindow* GetGLFWWindow() { return mWindow; }
		void SetGLFWKeyCallback(GLFWkeyfun);

	protected:
		void InitializeWindow() override;
		void InitializImGui();
	};
}
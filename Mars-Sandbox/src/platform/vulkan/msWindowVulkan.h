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

		MsWindowVulkan(const MsWindowVulkan& w) = delete;
		MsWindowVulkan operator=(const MsWindowVulkan& w) = delete;

		inline GLFWwindow* GetGLFWWindow() { return mWindow; }
		void SetGLFWKeyCallback(GLFWkeyfun);

	protected:
		void InitializeWindow() override;
		void InitializImGui();
	};
}
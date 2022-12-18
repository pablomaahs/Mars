#pragma once

#include "platform/common/msWindow.h"

namespace ms
{
	class MsWindowGL : public MsWindow
	{
	public:
		MsWindowGL() : MsWindow()
		{
			InitializeWindow();
			InitializImGui();
		};

		MsWindowGL(unsigned int w, unsigned int h, std::string name)
			: MsWindow(w, h, name)
		{
			InitializeWindow();
			InitializImGui();
		};
		~MsWindowGL();

		MsWindowGL(const MsWindowGL& w) = delete;
		MsWindowGL operator=(const MsWindowGL& w) = delete;

		inline GLFWwindow* GetGLFWWindow() { return mWindow; }
		void SetGLFWKeyCallback(GLFWkeyfun);

	private:
		void InitializeWindow() override;
		void InitializImGui();
	};
}
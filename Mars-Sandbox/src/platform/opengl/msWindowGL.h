#pragma once

#include "GLFW/glfw3.h"

// std
#include <string>

#include "platform/common/msWindow.h"

namespace ms
{
	class MsWindowGL : public MsWindow
	{
	public:
		MsWindowGL(unsigned int w, unsigned int h, std::string name);
		~MsWindowGL();

		MsWindowGL(const MsWindowGL& w) = delete;
		MsWindowGL operator=(const MsWindowGL& w) = delete;

		inline GLFWwindow* GetGLFWWindow() { return mWindow; }
		void SetGLFWKeyCallback(GLFWkeyfun);

	private:
		void InitializeWindow();
		void InitializImGui();
	};
}
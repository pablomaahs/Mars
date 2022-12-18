#pragma once

#include "GLFW/glfw3.h"

// std
#include <string>

namespace ms
{
	static constexpr unsigned int DEFAULT_WIDTH  = 1024;
	static constexpr unsigned int DEFAULT_HEIGHT = 768;
	static constexpr const char*  DEFAULT_NAME   = "Sandbox";

	class MsWindow
	{
	public:
		MsWindow(unsigned int w = DEFAULT_WIDTH, unsigned int h = DEFAULT_HEIGHT, std::string name = DEFAULT_NAME)
			: mWidth(w), mHeight(h), mName(name), mWindow(nullptr) {}
		virtual ~MsWindow() {};

		MsWindow(const MsWindow& w) = delete;
		MsWindow operator=(const MsWindow& w) = delete;

		inline virtual GLFWwindow* GetGLFWWindow() const
		{
			return mWindow;
		};

		inline const unsigned int GetWidth() { return mWidth; }
		inline const unsigned int GetHeight() { return mHeight; }

	protected:
		virtual void InitializeWindow() = 0;

		const unsigned int mWidth;
		const unsigned int mHeight;
		const std::string mName;

		GLFWwindow* mWindow;
	};
}
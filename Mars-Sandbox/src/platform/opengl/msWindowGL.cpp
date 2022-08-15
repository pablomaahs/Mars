#include "mspch.h"

#include "glad/glad.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "platform/common/msWindow.h"
#include "msWindowGL.h"

namespace ms
{
	MsWindowGL::MsWindowGL(unsigned int w, unsigned int h, std::string name) :
		MsWindow(w, h, name)
	{
		InitializeWindow();
		InitializImGui();
	}

	MsWindowGL::~MsWindowGL()
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	void MsWindowGL::SetGLFWKeyCallback(GLFWkeyfun callback)
	{
		glfwSetKeyCallback(mWindow, callback);
	}

	void MsWindowGL::InitializeWindow()
	{
		// Setup OpenGL context with GLFW and Glad
		glfwSetErrorCallback([](int error, const char* description) {
			std::cout << "Error: " << description << std::endl;
		});

		if (!glfwInit())
		{
			exit(EXIT_FAILURE);
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// TODO: Add support for full screen
		mWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);
		if (!mWindow)
		{
			exit(EXIT_FAILURE);
		}

		glfwMakeContextCurrent(mWindow);
		glfwSwapInterval(1);
		gladLoadGL();
	}

	void MsWindowGL::InitializImGui()
	{
		// ImGui Initialization
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
		ImGui_ImplOpenGL3_Init("#version 460 core");

		ImFontConfig cfg = ImFontConfig();
		cfg.FontDataOwnedByAtlas = false;
		cfg.RasterizerMultiply = 1.5f;
		cfg.SizePixels = 768.0f / 32.0f;
		cfg.PixelSnapH = true;
		cfg.OversampleH = 4;
		cfg.OversampleV = 4;

		//ImFont* font = io.Fonts->AddFontFromFileTTF("rsc/fonts/cour.ttf", cfg.SizePixels, &cfg);
		//IM_ASSERT(font != NULL);
	}
}
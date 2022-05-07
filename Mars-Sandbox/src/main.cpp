#include "mspch.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#pragma region Shader Source

static const char* shaderVertexSource = R"(
#version 460 core
layout (std140, location=0) uniform PerFrameData {
	uniform mat4 MVP;
};
layout (location=0) out vec2 uv;

const vec2 pos[3] = vec2[3](
	vec2(-0.6f, -0.4f),
	vec2( 0.6f, -0.4f),
	vec2( 0.0f,  0.6f)
);
const vec2 tc[3] = vec2[3](
	vec2( 0.0, 0.0 ),
	vec2( 1.0, 0.0 ),
	vec2( 0.5, 1.0 )
);

void main() {
	gl_Position = MVP * vec4(pos[gl_VertexID], 0.0, 1.0);
	uv = tc[gl_VertexID];
}
)";

static const char* shaderFragmentSource = R"(
#version 460 core
layout (location=0) in vec2 uv;
layout (location=0) out vec4 out_FragColor;
uniform sampler2D texture0;
void main()
{
	out_FragColor = texture(texture0, uv);
};
)";

#pragma endregion

int main()
{
	EASY_PROFILER_ENABLE;
	EASY_MAIN_THREAD;

	OPTICK_THREAD("Main Thread");
	OPTICK_START_CAPTURE();

	#pragma region Initialization

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

	GLFWwindow* window = glfwCreateWindow(1024, 768, "Sandbox", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	struct UserPointer {
		void* obj;
	} ptr;
	glfwSetWindowUserPointer(window, &ptr);

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
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
	});

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSwapInterval(1);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0f, -1.0f);

	// Preparing to Render
	GLuint VAO;
	glCreateVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	#pragma endregion

	#pragma region Shaders

	const GLuint shaderVertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shaderVertex, 1, &shaderVertexSource, nullptr);
	glCompileShader(shaderVertex);

	GLint isCompiled = 0;
	glGetShaderiv(shaderVertex, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shaderVertex, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(shaderVertex, maxLength, &maxLength, &infoLog[0]);

		std::cout << infoLog.data() << std::endl;
		exit(EXIT_FAILURE);
	}

	const GLuint shaderFragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shaderFragment, 1, &shaderFragmentSource, nullptr);
	glCompileShader(shaderFragment);

	glGetShaderiv(shaderFragment, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shaderFragment, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(shaderFragment, maxLength, &maxLength, &infoLog[0]);

		std::cout << infoLog.data() << std::endl;
		exit(EXIT_FAILURE);
	}

	const GLuint program = glCreateProgram();
	glAttachShader(program, shaderVertex);
	glAttachShader(program, shaderFragment);
	glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

		std::cout << infoLog.data() << std::endl;
		exit(EXIT_FAILURE);
	}

	glUseProgram(program);

	#pragma endregion

	struct PerFrameData {
		glm::mat4 mvp;
	};

	int w, h, comp;
	const uint8_t* img = stbi_load("rsc/textures/example/example.png", &w, &h, &comp, 3);

	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, 0);
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureStorage2D(texture, 1, GL_RGB8, w, h);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTextureSubImage2D(texture, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, img);
	glBindTextures(0, 1, &texture);

	stbi_image_free((void*)img);

	const GLsizeiptr kBufferSize = sizeof(PerFrameData);
	GLuint perFrameDataBuf;
	glCreateBuffers(1, &perFrameDataBuf);
	glNamedBufferStorage(perFrameDataBuf, kBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);

	#pragma region ImGui

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
	ImGui_ImplGlfw_InitForOpenGL(window, true);
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

	#pragma endregion

	// Main Loop
	while (!glfwWindowShouldClose(window))
	{
		EASY_BLOCK("Main loop");

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClearColor(.7f, .5f, .2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const float ratio = width / (float)height;
		const glm::mat4 m = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
		const glm::mat4 p = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);

		PerFrameData perFrameData = { p * m };
		glNamedBufferSubData(perFrameDataBuf, 0, kBufferSize, &perFrameData);
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameDataBuf, 0, kBufferSize);
		
		{
			EASY_BLOCK("Mars Rendering");
			OPTICK_PUSH("Mars Rendering");
			glDrawArrays(GL_TRIANGLES, 0, 3);
			OPTICK_POP();
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		ImGui::ShowDemoWindow();

		// Rendering
		{
			EASY_BLOCK("ImGui Rendering");
			OPTICK_PUSH("Mars Rendering");
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			OPTICK_POP();
		}

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	#pragma region Cleanup

	// Clean up
	glDeleteTextures(1, &texture);
	glDeleteBuffers(1, &perFrameDataBuf);
	glDeleteProgram(program);
	glDeleteShader(shaderFragment);
	glDeleteShader(shaderVertex);
	glDeleteVertexArrays(1, &VAO);

	glfwDestroyWindow(window);
	glfwTerminate();

	profiler::dumpBlocksToFile("profiler_data.prof");
	EASY_PROFILER_DISABLE;

	OPTICK_STOP_CAPTURE();
	OPTICK_SAVE_CAPTURE("profiler_dump");

	#pragma endregion

	return 0;
}
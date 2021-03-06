#include "mspch.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"
#include "meshoptimizer/src/meshoptimizer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#pragma region Shader Source

static const char* shaderVertexSource = R"(
#version 460 core
layout(std140, binding = 0) uniform PerFrameData
{
	uniform mat4 MVP;
	uniform int isWireframe;
};
layout (location=0) in vec3 pos;
layout (location=0) out vec3 color;
void main()
{
	gl_Position = MVP * vec4(pos, 1.0);
	color = isWireframe > 0 ? vec3(0.0f) : pos.xyz;
}
)";

static const char* shaderFragmentSource = R"(
#version 460 core
layout (location=0) in vec3 color;
layout (location=0) out vec4 out_FragColor;
void main()
{
	out_FragColor = vec4(color, 1.0);
};
)";

#pragma endregion

int main()
{
	EASY_PROFILER_ENABLE;
	EASY_MAIN_THREAD;

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
		int isWireframe;
	};

	const GLsizeiptr kBufferSize = sizeof(PerFrameData);

	GLuint perFrameDataBuf1;
	glCreateBuffers(1, &perFrameDataBuf1);
	glNamedBufferStorage(perFrameDataBuf1, kBufferSize * 2, nullptr, GL_DYNAMIC_STORAGE_BIT);

	GLuint perFrameDataBuf2;
	glCreateBuffers(1, &perFrameDataBuf2);
	glNamedBufferStorage(perFrameDataBuf2, kBufferSize * 2, nullptr, GL_DYNAMIC_STORAGE_BIT);

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

	#pragma region Assimp Bunny
	
	std::vector<glm::vec3> bunnyPositions;
	std::vector<unsigned int> bunnyIndices;

	{
		const aiScene* bunny = aiImportFile(
			"rsc/models/bunny/bunny.obj",
			aiProcess_Triangulate
		);

		if (!bunny || !bunny->HasMeshes())
		{
			std::cout << aiGetErrorString() << std::endl;
			exit(EXIT_FAILURE);
		}

		const aiMesh* mesh = bunny->mMeshes[0];
		for (unsigned i = 0; i != mesh->mNumFaces; i++)
		{
			for (int j = 0; j != 3; j++)
			{
				bunnyIndices.push_back(mesh->mFaces[i].mIndices[j]);
			}
		}

		for (unsigned i = 0; i != mesh->mNumVertices; i++)
		{
			const aiVector3D v = mesh->mVertices[i];
			bunnyPositions.push_back(glm::vec3(v.x, v.z, v.y));
		}

		aiReleaseImport(bunny);
	}

	// Mesh Optimezer - https://github.com/zeux/meshoptimizer/blob/master/README.md#pipeline
	// 1. Indexing - https://github.com/zeux/meshoptimizer/blob/master/README.md#indexing
	std::vector<unsigned int> bunnyRemap(bunnyIndices.size());
	const size_t bunnyVertexCount = meshopt_generateVertexRemap(bunnyRemap.data(), bunnyIndices.data(), bunnyIndices.size(), bunnyPositions.data(), bunnyPositions.size(), sizeof(glm::vec3));
	std::vector<unsigned int> remappedIndices(bunnyIndices.size());
	std::vector<glm::vec3> remappedVertices(bunnyVertexCount);
	meshopt_remapIndexBuffer(remappedIndices.data(), bunnyIndices.data(), bunnyIndices.size(), bunnyRemap.data());
	meshopt_remapVertexBuffer(remappedVertices.data(), bunnyPositions.data(), bunnyPositions.size(), sizeof(glm::vec3), bunnyRemap.data());
	// 2. Vertex cache optimization - https://github.com/zeux/meshoptimizer/blob/master/README.md#vertex-cache-optimization
	meshopt_optimizeVertexCache(remappedIndices.data(), remappedIndices.data(), bunnyIndices.size(), bunnyVertexCount);
	// 3. Overdraw optimization - https://github.com/zeux/meshoptimizer/blob/master/README.md#overdraw-optimization
	meshopt_optimizeOverdraw(remappedIndices.data(), remappedIndices.data(), bunnyIndices.size(), glm::value_ptr(remappedVertices[0]), bunnyVertexCount, sizeof(glm::vec3), 1.05f);
	// 4. Vertex fetch optimization - https://github.com/zeux/meshoptimizer/blob/master/README.md#vertex-fetch-optimization
	meshopt_optimizeVertexFetch(remappedVertices.data(), remappedIndices.data(), bunnyIndices.size(), glm::value_ptr(remappedVertices[0]), bunnyVertexCount, sizeof(glm::vec3));
	// 5. Simplification LOD - https://github.com/zeux/meshoptimizer/blob/master/README.md#simplification
	const float threshold = 0.2f;
	const size_t target_index_count = size_t(remappedIndices.size() * (size_t) threshold);
	const float target_error = 1e-2f;
	std::vector<unsigned int> idicesLod(remappedIndices.size());
	idicesLod.resize(
		meshopt_simplify(&idicesLod[0], remappedIndices.data(), remappedIndices.size(), glm::value_ptr(remappedVertices[0]), bunnyVertexCount, sizeof(glm::vec3), target_index_count, target_error)
	);

	bunnyIndices = remappedIndices;
	bunnyPositions = remappedVertices;

	const size_t sizeIndices = sizeof(unsigned int) * bunnyIndices.size();
	const size_t sizeIndicesLod = sizeof(unsigned int) * idicesLod.size();
	const size_t sizeVertices = sizeof(glm::vec3) * bunnyPositions.size();

	// We can store Indices and Vertices in the same buffer as follows:
	GLuint meshData;
	glCreateBuffers(1, &meshData);
	glNamedBufferStorage(meshData, sizeIndices + sizeIndicesLod + sizeVertices, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(meshData, 0							, sizeIndices		, bunnyIndices.data());
	glNamedBufferSubData(meshData, sizeIndices					, sizeIndicesLod	, idicesLod.data());
	glNamedBufferSubData(meshData, sizeIndices + sizeIndicesLod	, sizeVertices		, bunnyPositions.data());

	glVertexArrayElementBuffer(VAO, meshData);
	glVertexArrayVertexBuffer(VAO, 0, meshData, sizeIndices + sizeIndicesLod, sizeof(glm::vec3));
	glEnableVertexArrayAttrib(VAO, 0);
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(VAO, 0, 0);

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

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		//ImGui::ShowDemoWindow();

		// 2. Properties
		bool show_window = false;
		ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Always);
		ImGui::Begin("Properties 1", &show_window, ImGuiWindowFlags_NoResize);
		ImGui::LabelText("", "Transform");
		static float posVec3[3] = { 0.8f, -0.8f, -2.5f };
		ImGui::InputFloat3("Position", posVec3);
		static float rotVec3[3] = { -80.0f, 0.0f, 145.0f };
		ImGui::InputFloat3("Rotation", rotVec3);
		static float scaVec3[3] = { 0.7f, 0.7f, 0.7f };
		ImGui::InputFloat3("Scale", scaVec3);
		ImGui::Separator();
		ImGui::End();

		ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Always);
		ImGui::Begin("Properties 2", &show_window, ImGuiWindowFlags_NoResize);
		ImGui::LabelText("", "Transform");
		static float posVec3_2[3] = { -0.8f, -0.8f, -2.5f };
		ImGui::InputFloat3("Position", posVec3_2);
		static float rotVec3_2[3] = { -80.0f, 0.0f, 145.0f };
		ImGui::InputFloat3("Rotation", rotVec3_2);
		static float scaVec3_2[3] = { 0.7f, 0.7f, 0.7f };
		ImGui::InputFloat3("Scale", scaVec3_2);
		ImGui::Separator();
		ImGui::End();

		// Rendering
		{
			EASY_BLOCK("ImGui Rendering");
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

		glm::mat4 m1 = glm::translate(glm::mat4(1.0f), glm::vec3(posVec3[0], posVec3[1], posVec3[2]));
		rotVec3[2] = (float)glfwGetTime() * 20;
		m1 = glm::rotate(m1, glm::radians(rotVec3[0]), glm::vec3(1.0f, 0.0f, 0.0f));
		m1 = glm::rotate(m1, glm::radians(rotVec3[1]), glm::vec3(0.0f, 1.0f, 0.0f));
		m1 = glm::rotate(m1, glm::radians(rotVec3[2]), glm::vec3(0.0f, 0.0f, 1.0f));
		m1 = glm::scale(m1, glm::vec3(scaVec3[0], scaVec3[1], scaVec3[2]));

		glm::mat4 m2 = glm::translate(glm::mat4(1.0f), glm::vec3(posVec3_2[0], posVec3_2[1], posVec3_2[2]));
		rotVec3_2[2] = (float)glfwGetTime() * 20;
		m2 = glm::rotate(m2, glm::radians(rotVec3_2[0]), glm::vec3(1.0f, 0.0f, 0.0f));
		m2 = glm::rotate(m2, glm::radians(rotVec3_2[1]), glm::vec3(0.0f, 1.0f, 0.0f));
		m2 = glm::rotate(m2, glm::radians(rotVec3_2[2]), glm::vec3(0.0f, 0.0f, 1.0f));
		m2 = glm::scale(m2, glm::vec3(scaVec3_2[0], scaVec3_2[1], scaVec3_2[2]));

		const float ratio = width / (float)height;
		const glm::mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);

		PerFrameData perFrameData1[2] = { { p * m1 , false }, { p * m1 , true } };
		glNamedBufferSubData(perFrameDataBuf1, 0, kBufferSize * 2, &perFrameData1[0]);

		PerFrameData perFrameData2[2] = { { p * m2 , false }, { p * m2 , true } };
		glNamedBufferSubData(perFrameDataBuf2, 0, kBufferSize * 2, &perFrameData2[0]);
		
		{
			EASY_BLOCK("Mars Rendering");

			{
				EASY_BLOCK("Full Object");

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameDataBuf1, 0, kBufferSize);
				glDrawElements(GL_TRIANGLES, bunnyIndices.size(), GL_UNSIGNED_INT, 0);

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameDataBuf1, kBufferSize, kBufferSize);
				glDrawElements(GL_TRIANGLES, bunnyIndices.size(), GL_UNSIGNED_INT, 0);
			}

			{
				EASY_BLOCK("LOD Object");

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameDataBuf2, 0, kBufferSize);
				glDrawElements(GL_TRIANGLES, idicesLod.size(), GL_UNSIGNED_INT, (void*)sizeIndices);

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameDataBuf2, kBufferSize, kBufferSize);
				glDrawElements(GL_TRIANGLES, idicesLod.size(), GL_UNSIGNED_INT, (void*)sizeIndices);
			}
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	#pragma region Cleanup

	// Clean up
	glDeleteBuffers(1, &perFrameDataBuf1);
	glDeleteBuffers(1, &perFrameDataBuf2);
	glDeleteBuffers(1, &meshData);
	glDeleteProgram(program);
	glDeleteShader(shaderFragment);
	glDeleteShader(shaderVertex);
	glDeleteVertexArrays(1, &VAO);

	glfwDestroyWindow(window);
	glfwTerminate();

	profiler::dumpBlocksToFile("profiler_data.prof");
	EASY_PROFILER_DISABLE;

	#pragma endregion

	return 0;
}

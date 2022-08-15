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

#include "gltrace/GL.h"
#include "glUtils/GLShader.h"
#include "platform/opengl/msWindowGL.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

const std::string SHADER_PATH = "./rsc/shaders/";

int main()
{
	EASY_PROFILER_ENABLE;
	EASY_MAIN_THREAD;

	ms::MsWindowGL window(1024, 768, "OpenGL Window");

	window.SetGLFWKeyCallback([](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}
	});

	GL4API api;
	GetAPI4(&api, [](const char* func) -> void* { return (void*)glfwGetProcAddress(func); });
	InjectAPITracer4(&api);

	api.glEnable(GL_DEPTH_TEST);
	api.glEnable(GL_POLYGON_OFFSET_LINE);
	api.glPolygonOffset(-1.0f, -1.0f);

	// Preparing to Render
	GLuint VAO;
	api.glCreateVertexArrays(1, &VAO);
	api.glBindVertexArray(VAO);

	#pragma region Shaders
	GLShader vertexShader = GLShader((SHADER_PATH + "default.vert").c_str());
	GLShader fragmentShader = GLShader((SHADER_PATH + "default.frag").c_str());

	GLProgram program = GLProgram(vertexShader, fragmentShader);
	program.useProgram();
	#pragma endregion

	struct PerFrameData {
		glm::mat4 mvp;
		int isWireframe;
	};

	const GLsizeiptr kBufferSize = sizeof(PerFrameData);

	GLuint perFrameDataBuf;
	api.glCreateBuffers(1, &perFrameDataBuf);
	api.glNamedBufferStorage(perFrameDataBuf, kBufferSize * 2, nullptr, GL_DYNAMIC_STORAGE_BIT);

	#pragma region Assimp Bunny

	std::vector<glm::vec3> bunnyPositions;
	std::vector<unsigned int> bunnyIndices;

	{
		EASY_BLOCK("Initialization");

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

		bunnyIndices = remappedIndices;
		bunnyPositions = remappedVertices;

	}

	const size_t sizeIndices = sizeof(unsigned int) * bunnyIndices.size();
	const size_t sizeVertices = sizeof(glm::vec3) * bunnyPositions.size();

	// We can store Indices and Vertices in the same buffer as follows:
	GLuint meshData;
	api.glCreateBuffers(1, &meshData);
	api.glNamedBufferStorage(meshData, sizeIndices + sizeVertices, nullptr, GL_DYNAMIC_STORAGE_BIT);
	api.glNamedBufferSubData(meshData, 0, sizeIndices, bunnyIndices.data());
	api.glNamedBufferSubData(meshData, sizeIndices, sizeVertices, bunnyPositions.data());

	api.glVertexArrayElementBuffer(VAO, meshData);
	api.glVertexArrayVertexBuffer(VAO, 0, meshData, sizeIndices, sizeof(glm::vec3));
	api.glEnableVertexArrayAttrib(VAO, 0);
	api.glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	api.glVertexArrayAttribBinding(VAO, 0, 0);

	#pragma endregion

	// Main Loop
	while (!glfwWindowShouldClose(window.GetGLFWWindow()))
	{
		EASY_BLOCK("Main loop");

		glfwPollEvents();

		int width, height;
		glfwGetFramebufferSize(window.GetGLFWWindow(), &width, &height);
		api.glViewport(0, 0, width, height);
		api.glClearColor(.7f, .5f, .2f, 1.0f);
		api.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
		static float posVec3[3] = { 0.0f, -0.8f, -2.5f };
		ImGui::InputFloat3("Position", posVec3);
		static float rotVec3[3] = { -80.0f, 0.0f, 145.0f };
		ImGui::InputFloat3("Rotation", rotVec3);
		static float scaVec3[3] = { 1.f, 1.f, 1.f };
		ImGui::InputFloat3("Scale", scaVec3);
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
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(posVec3[0], posVec3[1], posVec3[2]));
		rotVec3[2] = (float)glfwGetTime() * 20;
		m = glm::rotate(m, glm::radians(rotVec3[0]), glm::vec3(1.0f, 0.0f, 0.0f));
		m = glm::rotate(m, glm::radians(rotVec3[1]), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::rotate(m, glm::radians(rotVec3[2]), glm::vec3(0.0f, 0.0f, 1.0f));
		m = glm::scale(m, glm::vec3(scaVec3[0], scaVec3[1], scaVec3[2]));

		const float ratio = width / (float)height;
		const glm::mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);

		PerFrameData perFrameData[2] = { { p * m , false }, { p * m , true } };
		api.glNamedBufferSubData(perFrameDataBuf, 0, kBufferSize * 2, &perFrameData[0]);
		
		{
			EASY_BLOCK("Mars Rendering");

			{
				EASY_BLOCK("Full Object");

				api.glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				api.glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameDataBuf, 0, kBufferSize);
				api.glDrawElements(GL_TRIANGLES, bunnyIndices.size(), GL_UNSIGNED_INT, 0);

				api.glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				api.glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameDataBuf, kBufferSize, kBufferSize);
				api.glDrawElements(GL_TRIANGLES, bunnyIndices.size(), GL_UNSIGNED_INT, 0);
			}
		}

		glfwSwapBuffers(window.GetGLFWWindow());
	}

	#pragma region Cleanup

	// Clean up
	api.glDeleteBuffers(1, &perFrameDataBuf);
	api.glDeleteBuffers(1, &meshData);
	api.glDeleteVertexArrays(1, &VAO);

	profiler::dumpBlocksToFile("profiler_data.prof");
	EASY_PROFILER_DISABLE;

	#pragma endregion

	return 0;
}

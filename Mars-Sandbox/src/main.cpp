#include "mspch.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "ext/matrix_transform.hpp"
#include "ext/matrix_clip_space.hpp"
#include "vec3.hpp"
#include "mat4x4.hpp"

#pragma region Shader Source

static const char* shaderVertexSource = R"(
#version 460 core
layout (std140, location=0) uniform PerFrameData {
	uniform mat4 MVP;
	uniform int isWireframe;
};
layout (location=0) out vec3 color;

const vec3 pos[8] = vec3[8] (
	vec3(-1.0,-1.0, 1.0), vec3(1.0, -1.0, 1.0),
	vec3(1.0, 1.0, 1.0), vec3(-1.0, 1.0, 1.0),

	vec3(-1.0,-1.0, -1.0), vec3(1.0,-1.0, -1.0),
	vec3( 1.0, 1.0, -1.0), vec3(-1.0, 1.0, -1.0)
);

const vec3 col[8] = vec3[8] (
	vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0), vec3(1.0, 1.0, 0.0),

	vec3(1.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0),
	vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0)
);

const int indices[36] = int[36] (
	// front
	0, 1, 2, 2, 3, 0,
	// right
	1, 5, 6, 6, 2, 1,
	// back
	7, 6, 5, 5, 4, 7,
	//left
	4, 0, 3, 3, 7, 4,
	// bottom
	4, 5, 1, 1, 0, 4,
	// top
	3, 2, 6, 6, 7, 3
);

void main() {
	int idx = indices[gl_VertexID];
	gl_Position = MVP * vec4(pos[idx], 1.0);
	color = isWireframe > 0 ? vec3(0.0) : col[idx];
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

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
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
	GLuint perFrameDataBuf;
	glCreateBuffers(1, &perFrameDataBuf);
	glNamedBufferStorage(perFrameDataBuf, 2 * kBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);

	// Main Loop
	while (!glfwWindowShouldClose(window))
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const float ratio = width / (float)height;
		const glm::mat4 m = glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(.0f, .0f, -3.5f)), (float)glfwGetTime(), glm::vec3(1.0f, 1.0f, 1.0f));
		const glm::mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);

		PerFrameData perFrameData[2] = { { p * m , false }, { p * m , true } };
		glNamedBufferSubData(perFrameDataBuf, 0, 2 * kBufferSize, &perFrameData);

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameDataBuf, 0, kBufferSize);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindBufferRange(GL_UNIFORM_BUFFER, 0, perFrameDataBuf, kBufferSize, kBufferSize);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	#pragma region Cleanup

	// Clean up
	glDeleteProgram(program);
	glDeleteShader(shaderFragment);
	glDeleteShader(shaderVertex);
	glDeleteVertexArrays(1, &VAO);

	glfwDestroyWindow(window);
	glfwTerminate();

	#pragma endregion

	return 0;
}
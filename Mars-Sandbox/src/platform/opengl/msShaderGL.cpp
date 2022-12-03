#include <mspch.h>

#include "msShaderGL.h"
#include "platform/common/utils/Utils.h"

namespace
{
	msShaderGL::msShaderGL(const char* fileName)
		: msShaderGL(shaderTypeFromFileName(fileName), readShaderFile(fileName).c_str(), fileName)
	{}

	msShaderGL::msShaderGL(GLenum type, const char* text, const char* debugFileName)
		: type_(type)
		, handle_(glCreateShader(type))
	{
		glShaderSource(handle_, 1, &text, nullptr);
		glCompileShader(handle_);

		char buffer[8192];
		GLsizei length = 0;
		glGetShaderInfoLog(handle_, sizeof(buffer), &length, buffer);

		if (length)
		{
			printf("%s (File: %s)\n", buffer, debugFileName);
			printShaderSource(text);
			assert(false);
		}
	}

	msShaderGL::~msShaderGL()
	{
		glDeleteShader(handle_);
	}

	void printProgramInfoLog(GLuint handle)
	{
		char buffer[8192];
		GLsizei length = 0;
		glGetProgramInfoLog(handle, sizeof(buffer), &length, buffer);
		if (length)
		{
			printf("%s\n", buffer);
			assert(false);
		}
	}

	GLProgram::GLProgram(const msShaderGL& a)
		: handle_(glCreateProgram())
	{
		glAttachShader(handle_, a.getHandle());
		glLinkProgram(handle_);
		printProgramInfoLog(handle_);
	}

	GLProgram::GLProgram(const msShaderGL& a, const msShaderGL& b)
		: handle_(glCreateProgram())
	{
		glAttachShader(handle_, a.getHandle());
		glAttachShader(handle_, b.getHandle());
		glLinkProgram(handle_);
		printProgramInfoLog(handle_);
	}

	GLProgram::GLProgram(const msShaderGL& a, const msShaderGL& b, const msShaderGL& c)
		: handle_(glCreateProgram())
	{
		glAttachShader(handle_, a.getHandle());
		glAttachShader(handle_, b.getHandle());
		glAttachShader(handle_, c.getHandle());
		glLinkProgram(handle_);
		printProgramInfoLog(handle_);
	}

	GLProgram::GLProgram(const msShaderGL& a, const msShaderGL& b, const msShaderGL& c, const msShaderGL& d, const msShaderGL& e)
		: handle_(glCreateProgram())
	{
		glAttachShader(handle_, a.getHandle());
		glAttachShader(handle_, b.getHandle());
		glAttachShader(handle_, c.getHandle());
		glAttachShader(handle_, d.getHandle());
		glAttachShader(handle_, e.getHandle());
		glLinkProgram(handle_);
		printProgramInfoLog(handle_);
	}

	GLProgram::~GLProgram()
	{
		glDeleteProgram(handle_);
	}

	void GLProgram::useProgram() const
	{
		glUseProgram(handle_);
	}

	GLenum shaderTypeFromFileName(const char* fileName)
	{
		if (endsWith(fileName, ".vert"))
			return GL_VERTEX_SHADER;

		if (endsWith(fileName, ".frag"))
			return GL_FRAGMENT_SHADER;

		if (endsWith(fileName, ".geom"))
			return GL_GEOMETRY_SHADER;

		if (endsWith(fileName, ".tesc"))
			return GL_TESS_CONTROL_SHADER;

		if (endsWith(fileName, ".tese"))
			return GL_TESS_EVALUATION_SHADER;

		if (endsWith(fileName, ".comp"))
			return GL_COMPUTE_SHADER;

		assert(false);

		return 0;
	}
}
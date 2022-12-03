#pragma once

#include <glad/glad.h>

namespace
{
	class msShaderGL
	{
	public:
		explicit msShaderGL(const char* fileName);
		msShaderGL(GLenum type, const char* text, const char* debugFileName = "");
		~msShaderGL();
		GLenum getType() const { return type_; }
		GLuint getHandle() const { return handle_; }

	private:
		GLenum type_;
		GLuint handle_;
	};

	class GLProgram
	{
	public:
		GLProgram(const msShaderGL& a);
		GLProgram(const msShaderGL& a, const msShaderGL& b);
		GLProgram(const msShaderGL& a, const msShaderGL& b, const msShaderGL& c);
		GLProgram(const msShaderGL& a, const msShaderGL& b, const msShaderGL& c, const msShaderGL& d, const msShaderGL& e);
		~GLProgram();

		void useProgram() const;
		GLuint getHandle() const { return handle_; }

	private:
		GLuint handle_;
	};

	GLenum shaderTypeFromFileName(const char* fileName);
}

#include "Technique.hpp"

#include <fstream>
#include <sstream>

#include <GL/glew.h>

namespace Rendering
{
	Technique::Technique()
	{
		ProgramIndex = glCreateProgram();
	}

	Technique::~Technique()
	{
		for (int shaderIndex : Shaders)
		{
			glDeleteShader(shaderIndex);
		}

		if (ProgramIndex != 0)
		{
			glDeleteProgram(ProgramIndex);
		}
	}

	bool Technique::AddShader(int shaderType, const std::string& filename)
	{
		std::string shaderText;
		std::ifstream shaderFile(filename);
		std::stringstream ss;
		ss << shaderFile.rdbuf();
		shaderText = ss.str();
		const char* textPointer = shaderText.c_str();
		int length = shaderText.size();

		GLuint ShaderObj = glCreateShader(shaderType);

		if (ShaderObj == 0) {
			fprintf(stderr, "Error creating shader type %d\n", shaderType);
			return false;
		}

		Shaders.push_back(ShaderObj);

		glShaderSource(ShaderObj, 1, &textPointer, &length);
		glCompileShader(ShaderObj);

		GLint success;
		glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

		if (!success) {
			GLchar InfoLog[1024];
			glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
			fprintf(stderr, "Error compiling '%s': '%s'\n", filename.c_str(), InfoLog);
			return false;
		}

		glAttachShader(ProgramIndex, ShaderObj);

		return true;
	}

	bool Technique::Finalize()
	{
		GLint Success = 0;
		GLchar ErrorLog[1024] = { 0 };

		glLinkProgram(ProgramIndex);

		glGetProgramiv(ProgramIndex, GL_LINK_STATUS, &Success);
		if (Success == 0) {
			glGetProgramInfoLog(ProgramIndex, sizeof(ErrorLog), NULL, ErrorLog);
			fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
			return false;
		}

		glValidateProgram(ProgramIndex);
		glGetProgramiv(ProgramIndex, GL_VALIDATE_STATUS, &Success);
		if (!Success) {
			glGetProgramInfoLog(ProgramIndex, sizeof(ErrorLog), NULL, ErrorLog);
			fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
			return false;
		}

		// Delete the intermediate shader objects that have been added to the program
		for (int shaderIndex : Shaders) {
			glDeleteShader(shaderIndex);
		}

		Shaders.clear();

		return glGetError() == 0;
	}

	void Technique::Enable()
	{
		glUseProgram(ProgramIndex);
	}
}
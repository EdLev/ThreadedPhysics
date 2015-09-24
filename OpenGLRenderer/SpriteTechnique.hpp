#pragma once

#include <GL/glew.h>
#include <GL/wglew.h>

#include "../Core/Matrix4.hpp"
#include "Technique.hpp"

struct SphereSprite
{
	Core::Vector4 Position;
	Core::Vector4 Color;
	float Radius;
};

class SpriteTechnique : public Rendering::Technique
{
public:
	SpriteTechnique()
	{
		Setup();
	}

	virtual void Enable()
	{

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		Technique::Enable();
	}

	void SetViewProjection(const Core::Matrix4& viewProjection)
	{
		glUniformMatrix4fv(ViewProjectionIndex, 1, GL_FALSE, viewProjection.begin()->begin());
	}
	void SetCameraPosition(const Core::Vector4& cameraPosition)
	{
		glUniform4fv(CameraPositionIndex, 1, cameraPosition.begin());
	}

protected:
	virtual bool Setup()
	{
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		AddShader(GL_VERTEX_SHADER, "SphereParticles.vs");
		AddShader(GL_GEOMETRY_SHADER, "SphereParticles.gs");
		AddShader(GL_FRAGMENT_SHADER, "SphereParticles.fs");

		glEnable(GL_PROGRAM_POINT_SIZE);

		Finalize();

		ViewProjectionIndex = glGetUniformLocation(ProgramIndex, "gVP");
		CameraPositionIndex = glGetUniformLocation(ProgramIndex, "gCameraPos");

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		return true;
	}


private:

	int ViewProjectionIndex;
	int CameraPositionIndex;
};
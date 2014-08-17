#version 330

layout (location = 0) in vec4 Position;
layout (location = 1) in vec4 InColor;
layout (location = 2) in float Radius;

out vec4 VertexColor;

void main()
{
    gl_Position = Position;
	gl_PointSize = Radius;
	VertexColor = InColor;
}
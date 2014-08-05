#version 330

layout (location = 0) in vec4 Position;
layout (location = 1) in float Radius;

out float OutRadius;

void main()
{
    gl_Position = vec4(Position.xyz, 1.0);
	gl_PointSize = Radius;
}
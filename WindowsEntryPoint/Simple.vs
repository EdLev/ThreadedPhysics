#version 330

layout (location = 0) in vec4 Position;

uniform mat4 ViewProjection;

void main()
{
    gl_Position = ViewProjection * vec4(Position.xyz, 1.0);
}
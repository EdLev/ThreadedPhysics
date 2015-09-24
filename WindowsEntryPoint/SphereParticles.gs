#version 330

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform mat4 gVP;
uniform vec4 gCameraPos;

in vec4 VertexColor[];

out vec4 Color;
out vec2 TexCoord;

out float Depth;

void main()
{
	vec4 Pos = vec4(gl_in[0].gl_Position.xyz, 1.0);
	float Size = gl_in[0].gl_PointSize;

	vec4 toCamera = normalize(gCameraPos - -Pos);
	vec4 up = vec4(0.0, 1.0, 0.0, 0.0);
	vec4 right = vec4(normalize(cross(up.xyz, toCamera.xyz)),0.0);
	up = vec4(normalize(cross(toCamera.xyz, right.xyz)),0.0f);

	Pos -= right * Size;
	Pos += up * Size;
	gl_Position = gVP * Pos;
	TexCoord = vec2(0.0, 0.0);
	Color = VertexColor[0];
	Depth = Pos.z / 1000;
	EmitVertex();

	Pos += 2.0 * right * Size;
	gl_Position = gVP * Pos;
	TexCoord = vec2(0.0, 1.0);
	Color = VertexColor[0];
	Depth = Pos.z / 1000;
	EmitVertex();

	Pos -= 2.0 * up * Size;
	Pos -= 2.0 * right * Size;
	gl_Position = gVP * Pos;
	TexCoord = vec2(1.0, 0.0);
	Color = VertexColor[0];
	Depth = Pos.z / 1000;
	EmitVertex();

	Pos += 2.0 * right * Size;
	gl_Position = gVP * Pos;
	TexCoord = vec2(1.0, 1.0);
	Color = VertexColor[0];
	Depth = Pos.z / 1000;
	EmitVertex();


	EndPrimitive();

}
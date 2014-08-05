#version 330

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform mat4 gVP;
uniform vec4 gCameraPos;

out vec2 TexCoord;
out vec4 WorldSpacePos;

void main()
{
	vec3 Pos = gl_in[0].gl_Position.xyz;
	float Size = gl_in[0].gl_PointSize;

	vec3 toCamera = normalize(gCameraPos.xyz - Pos);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = vec3(1.0,0.0,0.0);

	Pos -= (right * 0.5) * Size;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(0.0, 0.0);
	EmitVertex();

	Pos.y += 1.0 * Size;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(0.0, 1.0);
	EmitVertex();

	Pos.y -= 1.0 * Size;
	Pos += right * Size;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(1.0, 0.0);
	EmitVertex();

	Pos.y += 1.0 * Size;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();

}
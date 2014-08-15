#version 330

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform mat4 gVP;
uniform vec4 gCameraPos;

in vec4 VertexColor[];

out vec4 Color;
out vec2 TexCoord;

void main()
{
	vec3 Pos = gl_in[0].gl_Position.xyz;
	float Size = gl_in[0].gl_PointSize;

	//vec3 toCamera = vec3(0.0,0.0,1.0);//normalize(gCameraPos.xyz - Pos);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = vec3(1.0, 0.0, 0.0);//normalize(cross(up, toCamera));
	//up = normalize(cross(toCamera, right));

	Pos -= right * Size;
	Pos += up * Size;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(0.0, 0.0);
	Color = VertexColor[0];
	EmitVertex();

	Pos += 2.0 * right * Size;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(0.0, 1.0);
	Color = VertexColor[0];
	EmitVertex();

	Pos -= 2.0 * up * Size;
	Pos -= 2.0 * right * Size;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(1.0, 0.0);
	Color = VertexColor[0];
	EmitVertex();

	Pos += 2.0 * right * Size;
	gl_Position = gVP * vec4(Pos, 1.0);
	TexCoord = vec2(1.0, 1.0);
	Color = VertexColor[0];
	EmitVertex();


	EndPrimitive();

}
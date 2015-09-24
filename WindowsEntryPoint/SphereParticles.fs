#version 330

in vec2 TexCoord;
in vec4 Color;
in float Depth;

out vec4 FragColor;

void main()
{
    vec2 pos = (TexCoord - 0.5) * 2.0;
	float dist = dot(pos, pos);

	if (dist > 1.0) 
	{
		discard;
	}

	FragColor = Color;
} 
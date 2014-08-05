#version 330

in vec2 TexCoord;
out vec4 FragColor;

void main()
{

    vec2 pos = ( TexCoord - 0.5 ) * 2.0;
    float dist = dot( pos, pos );

   if (dist > 1.0) 
   {
        discard;
   }

    FragColor = vec4( 1.0 - dist, 0.0, 0.0, 1.0 );
} 
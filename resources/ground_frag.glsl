#version 330 core 

uniform sampler2D Texture0;

in vec2 vTexCoord;
in float dCo;
out vec4 color;

void main()
{
	vec4 texColor0 = texture(Texture0, vTexCoord);
	
	color = dCo*texColor0;
}
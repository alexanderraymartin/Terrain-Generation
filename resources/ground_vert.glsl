#version  330 core

layout(location = 0) in vec4 vertPos;

uniform mat4 P;
uniform mat4 MV;
uniform mat4 view;

out vec3 fragNor;

void main()
{
	gl_Position = P * view * MV * vertPos;
	fragNor = (MV * vec4(vertPos.xyz, 0.0)).xyz;
}

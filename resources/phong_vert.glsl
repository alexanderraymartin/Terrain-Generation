#version  330 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 MV;
uniform mat4 view;

out vec3 fragNor;

void main()
{
	gl_Position = P * view * MV * vertPos;
	fragNor = (MV * vec4(vertNor, 0.0)).xyz;
}

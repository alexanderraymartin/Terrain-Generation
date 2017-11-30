#version  330 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P;
uniform mat4 MV;
uniform mat4 view;
uniform float light_x_position;

out float dCo;
out vec2 vTexCoord;
out vec3 fragNor;

void main()
{
	vec3 lightPosition = normalize(vec3(light_x_position, 100.0, 0.0));
	
	gl_Position = P * view * MV * vertPos;
	fragNor = (MV * vec4(vertNor, 0.0)).xyz;
	dCo = max(dot(fragNor, normalize(lightPosition)), 0);
	vTexCoord = vertTex;
}
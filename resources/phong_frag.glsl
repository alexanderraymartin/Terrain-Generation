#version 330 core 

uniform float light_x_position;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;

in vec3 fragNor;
out vec4 color;

void main()
{
	vec3 lightPosition = normalize(vec3(light_x_position, 1.0, 1.0));
	vec3 normal = normalize(fragNor);
	vec3 reflection = reflect(-lightPosition, normal);
	
	vec3 diffuse = MatDif * max(0.0, dot(normal, lightPosition));
	vec3 specular = MatSpec * pow(max(0.0, dot(reflection, normal)), shine);
	vec3 ambient = MatAmb;

	color = vec4(diffuse + specular + ambient, 1.0);
}
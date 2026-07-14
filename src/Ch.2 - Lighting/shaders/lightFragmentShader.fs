#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambientStrength;

uniform vec3 viewPos;
uniform float specularStrength;

void main()
{
	// ambient lighting
	vec3 ambient = ambientStrength * lightColor;

	// diffuse lighting
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * lightColor;

	// specular lighting
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
	vec3 specular = specularStrength * spec * lightColor;

	vec4 texColor = texture(texture_diffuse1, TexCoord) * vec4(ambient + diffuse + specular, 1.0f);
	FragColor = texColor;
}

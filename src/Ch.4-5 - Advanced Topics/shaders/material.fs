#version 330 core

struct Material {
	sampler2D texture_diffuse1; // texture for color under diffuse lighting
	sampler2D texture_specular1; // texture for color of specular highlight on material
	sampler2D texture_normal1;
	float shininess; // impacts scattering/radius of specular highlight

	bool hasDiffuse;
	bool hasSpecular;
	bool hasNormal;

	bool unlit;
};

struct DirectionalLight {
	bool enabled;

	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

#define NR_POINT_LIGHTS 1

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

// clipping planes
uniform float near;
uniform float far;

uniform Material material;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform vec3 viewPos;

float LinearizeDepth(float depth);
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	if (material.unlit)
	{
		FragColor = texture(material.texture_diffuse1, TexCoord);
	}
	else
	{
		vec3 norm = normalize(Normal);
		vec3 viewDir = normalize(viewPos - FragPos);

		vec3 result = vec3(0.0f);
		if (directionalLight.enabled)
		{
			result += CalcDirLight(directionalLight, norm, viewDir);
		}
	
		for (int i = 0; i < NR_POINT_LIGHTS; i++)
		{
			result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
		}

		// applies fog color to far away objects
		float depth = LinearizeDepth(gl_FragCoord.z) / far;
		vec3 fogColor = vec3(0.5f, 0.5f, 0.34f);
		result -= (depth * (vec3(1.0f)-fogColor));

		vec4 texColor = texture(material.texture_diffuse1, TexCoord);
		FragColor = vec4(result, texColor.w);
	}
}

float LinearizeDepth(float depth)
{
	// converts depth value to native device coordinates in range [-1, 1]
	float z = depth * 2.0 - 1.0;
	// retrieves linear depth value from the nonlinear depth value via inversion
	return (2.0 * near * far) / (far + near - z * (far - near));
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	
	// ambient lighting
	vec3 diffuseColor = material.hasDiffuse ? vec3(texture(material.texture_diffuse1, TexCoord)) : vec3(1.0f, 0.0f, 0.0f); // sets diffuse color to red if no texture
	vec3 ambient = light.ambient * diffuseColor;

	// diffuse lighting
	float diff = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = light.diffuse * diff * diffuseColor;

	// specular lighting
	vec3 specularColor = material.hasSpecular ? vec3(texture(material.texture_specular1, TexCoord)) : vec3(0.0f); // sets specularColor to black if no texture
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
	vec3 specular = light.specular * spec * specularColor;

	vec3 result = ambient + diffuse + specular;
	return result;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);

	// ambient lighting
	vec3 diffuseColor = material.hasDiffuse ? vec3(texture(material.texture_diffuse1, TexCoord)) : vec3(1.0f, 0.0f, 0.0f); // sets diffuse color to red if no texture
	vec3 ambient = light.ambient * diffuseColor;

	// diffuse lighting
	float diff = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = light.diffuse * diff * diffuseColor;

	// specular lighting
	vec3 specularColor = material.hasSpecular ? vec3(texture(material.texture_specular1, TexCoord)) : vec3(0.0f); // sets specularColor to black if no texture
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
	vec3 specular = light.specular * spec * specularColor;

	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	vec3 result = ambient + diffuse + specular;
	return result;
}

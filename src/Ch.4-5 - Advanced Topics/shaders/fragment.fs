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

	// defines area that directionalLight affects
	vec3 center;
	float innerRadius;
	float outerRadius;

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

#define NR_POINT_LIGHTS 3

in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;
in mat3 TBN;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform sampler2D shadowMap;

// clipping planes
uniform float near;
uniform float far;

uniform Material material;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace);
float LinearizeDepth(float depth);
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 result = vec3(0.0f);

	if (material.unlit)
	{
		FragColor = texture(material.texture_diffuse1, TexCoord);
	}

	else
	{
		vec3 normal = normalize(Normal);
		normal = texture(material.texture_normal1, TexCoord).rgb;
		normal.g = 1.0f - normal.g;
		normal = normalize(normal * 2.0f - 1.0f);
		normal = normalize(TBN * normal);
		
		vec3 viewDir = normalize(viewPos - FragPos);

		if (length(FragPos - directionalLight.center) < directionalLight.outerRadius)
		{		
			result += CalcDirLight(directionalLight, normal, FragPos, viewDir);
		}
	
		for (int i = 0; i < NR_POINT_LIGHTS; i++)
		{
			result += CalcPointLight(pointLights[i], normal, FragPos, viewDir);
		}

		// applies fog color to far away objects
		float depth = LinearizeDepth(gl_FragCoord.z) / far;
		//float depth = gl_FragCoord.z / 10;
		vec3 fogColor = vec3(0.13f, 0.13f, 0.13f);
		result = mix(result, fogColor, depth);

		vec4 texColor = texture(material.texture_diffuse1, TexCoord);
		FragColor = vec4(result, texColor.a);
	}
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
	// performs perspective division
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transforms to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	// get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;
	// applies bias to shadowing to correct artifacting (more bias for higher angles between light direction and surface normal)
	// float bias = max(0.05f * (1.0) - dot(normal, lightDir)), 0.005);
	float bias = 0.005f;
	// determines if fragment is in shadow
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

	return shadow;
}

float LinearizeDepth(float depth)
{
	// converts depth value to native device coordinates in range [-1, 1]
	float z = depth * 2.0 - 1.0;
	// retrieves linear depth value from the nonlinear depth value via inversion
	return (2.0 * near * far) / (far + near - z * (far - near));
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
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
	float spec = 0.0f;
	// blinn-phong specular highlight
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess * 2);
	// phong specular highlight
	// vec3 reflectDir = reflect(-lightDir, normal);
	// spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = light.specular * spec * specularColor;

	// blend from inner to outer radius
	if (length(fragPos - directionalLight.center) > directionalLight.innerRadius)
	{		
		float distance = length(fragPos - directionalLight.center);
		float intensity = (directionalLight.outerRadius - distance) / (directionalLight.outerRadius - directionalLight.innerRadius);
		ambient *= intensity;
		diffuse *= intensity;
		specular *= intensity;
	}

	//float shadow = ShadowCalculation(FragPosLightSpace);
	float shadow = 0.0f;
	vec3 result = ambient + (1.0f - shadow) * (diffuse + specular);

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
	float spec = 0.0f;
	// blinn-phong specular highlight
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess * 2);
	// phong specular highlight
	// vec3 reflectDir = reflect(-lightDir, normal);
	// spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
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

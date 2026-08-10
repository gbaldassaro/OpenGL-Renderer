#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct Material {
	sampler2D texture_diffuse1; // texture for color under diffuse lighting
	sampler2D texture_specular1; // texture for color of specular highlight on material
	sampler2D texture_normal1;
	float shininess; // impacts scattering/radius of specular highlight
	float opacity;

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

#define NR_DIRECTIONAL_LIGHTS 1
#define NR_POINT_LIGHTS 9

in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;
in mat3 TBN;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

uniform sampler2D depthMap;

uniform float texOffset;
uniform bool useOpacityGradient;

// clipping planes
uniform float near;
uniform float far;

uniform Material material;
uniform DirectionalLight directionalLights[NR_DIRECTIONAL_LIGHTS];
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
		vec4 texColor = texture(material.texture_diffuse1, vec2(mod(TexCoord.x + texOffset, 1.0f), TexCoord.y));
		texColor.a *= material.opacity;
		FragColor = texColor;
	}

	else
	{
		vec3 normal = normalize(Normal);
		normal = texture(material.texture_normal1, vec2(mod(TexCoord.x + texOffset, 1.0f), TexCoord.y)).rgb;
		normal.g = 1.0f - normal.g;
		normal = normalize(normal * 2.0f - 1.0f);
		normal = normalize(TBN * normal);
		
		vec3 viewDir = normalize(viewPos - FragPos);

		for (int i = 0; i < NR_DIRECTIONAL_LIGHTS; i++)
		{
			if (length(FragPos - directionalLights[i].center) < directionalLights[i].outerRadius)
			{		
				result += CalcDirLight(directionalLights[i], normal, FragPos, viewDir);
			}
		}
	
		for (int i = 0; i < NR_POINT_LIGHTS; i++)
		{
			result += CalcPointLight(pointLights[i], normal, FragPos, viewDir);
		}

		// applies fog color to far away objects
		float depth = LinearizeDepth(gl_FragCoord.z) / far;
		//float depth = gl_FragCoord.z / 10;
		vec3 fogColor = vec3(0.05f, 0.05f, 0.05f);
		result = mix(result, fogColor, depth);

		vec4 texColor = texture(material.texture_diffuse1, vec2(mod(TexCoord.x + texOffset, 1.0f), TexCoord.y));
		float opacity = texColor.a * material.opacity;

		if (useOpacityGradient)
		{
			//if (length(FragPos - vec3(0.0f, 33.0f, -62.0f) < 30.0f)
			//{		
			//	float distance = length(FragPos - vec3(0.0f, 33.0f, -62.0f);
			//	opacity *= (30.0f - distance) / (30.0f - 20.0f);
			//}
			opacity -= (FragPos.z / -130.0f);
		}

		FragColor = vec4(result, opacity);
	}

	// renders bright colors for bloom
	float brightness = dot(FragColor.rgb, vec3(0.2126f, 0.7152f, 0.0722f));
    if (brightness > 0.5f)
    {
        BrightColor = vec4(FragColor.rgb, 1.0f);
    }
	else
	{
		BrightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
	// performs perspective division
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// transforms to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(depthMap, projCoords.xy).r;
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
	vec3 diffuseColor = material.hasDiffuse ? vec3(texture(material.texture_diffuse1, vec2(mod(TexCoord.x + texOffset, 1.0f), TexCoord.y))) : vec3(1.0f, 0.0f, 0.0f); // sets diffuse color to red if no texture
	vec3 ambient = light.ambient * diffuseColor;

	// diffuse lighting
	float diff = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = light.diffuse * diff * diffuseColor;

	// specular lighting
	vec3 specularColor = material.hasSpecular ? vec3(texture(material.texture_specular1, vec2(mod(TexCoord.x + texOffset, 1.0f), TexCoord.y))) : vec3(0.0f); // sets specularColor to black if no texture
	float spec = 0.0f;
	// blinn-phong specular highlight
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess * 2);
	// phong specular highlight
	// vec3 reflectDir = reflect(-lightDir, normal);
	// spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = light.specular * spec * specularColor;

	// blend from inner to outer radius
	if (length(fragPos - light.center) > light.innerRadius)
	{		
		float distance = length(fragPos - light.center);
		float intensity = (light.outerRadius - distance) / (light.outerRadius - light.innerRadius);
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
	vec3 diffuseColor = material.hasDiffuse ? vec3(texture(material.texture_diffuse1, vec2(mod(TexCoord.x + texOffset, 1.0f), TexCoord.y))) : vec3(1.0f, 0.0f, 0.0f); // sets diffuse color to red if no texture
	vec3 ambient = light.ambient * diffuseColor;

	// diffuse lighting
	float diff = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = light.diffuse * diff * diffuseColor;

	// specular lighting
	vec3 specularColor = material.hasSpecular ? vec3(texture(material.texture_specular1, vec2(mod(TexCoord.x + texOffset, 1.0f), TexCoord.y))) : vec3(0.0f); // sets specularColor to black if no texture
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

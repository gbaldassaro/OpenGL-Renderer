#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform vec3 lightColor;

void main()
{
	vec4 texColor = texture(texture_diffuse1, TexCoord);// * vec4(lightColor, 1.0f);
	//if (texColor.a < 0.3f)
	//{
	//	discard;
	//}
	FragColor = texColor;
}

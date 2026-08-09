#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

// creates a depth map from a light's perspective for use in shadow mapping
void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}

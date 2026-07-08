#version 330 core

layout (location = 0) in vec3 aPos; // "in" keyword declares the input vertex attributes
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	vertexColor = aColor;
}

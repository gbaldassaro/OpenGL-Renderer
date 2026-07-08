#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader/shader.h>
#include<stb_image/stb_image.h>

#include <iostream>

// forward declarations of functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// setting variables
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
#pragma region Initialization
	// initialize and configure the GLFW window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// creates a window object
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// initialize GLAD to load OpenGL function pointers before calling any OpenGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
#pragma endregion

#pragma region Shader;
	Shader shader("Ch.1 - Getting Started/shaders/textureVertexShader.vs", "Ch.1 - Getting Started/shaders/textureFragmentShader.fs");
#pragma endregion

#pragma region Vertex
	// vertices of triangle, used with VAO and VBO
	//float vertices[] = {
	//  // positions         // colors
	//	-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, // bottom left
	//	 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // bottom right
	//	 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f  // top
	//};
	//// vertices of rectangle, used with EBO
	float vertices[] = {
		// positions		 // colors			// texture coords
		 0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, // top right
		 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = { 
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	// creates IDs 
	unsigned int VAO, VBO, EBO;
	// generates one vertex array using the ID of the vertex array
	glGenVertexArrays(1, &VAO);
	// generates one vertex buffer using the ID of the vertex buffer
	glGenBuffers(1, &VBO);
	// generates one element buffer using the ID of the element buffer
	glGenBuffers(1, &EBO);

	// binds vertex array
	glBindVertexArray(VAO);
	// binds the vertex buffer to the GL_ARRAY_BUFFER target
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// copies vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// binds the element buffer to the GL_ELEMENT_ARRAY_BUFFER target
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// copies indices into the buffer's memory
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// instructs how to read vertex attributes from the vertex buffer then enables it
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture coordinate
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// unbinds vertex buffer and vertex array 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
#pragma endregion

#pragma region Texture
	stbi_set_flip_vertically_on_load(true);

	// prepares variables
	int width, height, nrChannels;

	// texture 1 ----------------------------------------------------------------------------
	// loads in the texture image
	unsigned char* data = stbi_load("Ch.1 - Getting Started/textures/lovett_hall.jpg", &width, &height, &nrChannels, 0);

	// sets reference ID of the texture
	unsigned int texture1;
	glGenTextures(1, &texture1);

	// binds the texture to the target GL_TEXTURE_2D
	glBindTexture(GL_TEXTURE_2D, texture1);

	// sets options on the currently bound texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// generates texture using the image data
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	// frees the image memory
	stbi_image_free(data);


	// texture 2 ----------------------------------------------------------------------------
	data = stbi_load("Ch.1 - Getting Started/textures/rice_logo.png", &width, &height, &nrChannels, 0);

	unsigned int texture2;
	glGenTextures(1, &texture2);

	glBindTexture(GL_TEXTURE_2D, texture2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);
#pragma endregion

#pragma region Rendering
	// enables wireframe mode
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// activates shaders
	shader.use();

	// sets texture uniforms in shader
	shader.setInt("texture1", 0);
	shader.setInt("texture2", 1);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// user input
		processInput(window);

		// rendering commands
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // state-setting function
		glClear(GL_COLOR_BUFFER_BIT); // state-using function

		// assigns texture to the fragment shader's uniform sampler2D 
		glActiveTexture(GL_TEXTURE0); // activates texture unit before binding
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1); // activates texture unit before binding
		glBindTexture(GL_TEXTURE_2D, texture2);

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
		trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
		unsigned int transformLoc = glGetUniformLocation(shader.ID, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

		// binds vertex array
		glBindVertexArray(VAO);
		// draws when using VAO and VBO
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		// draws when using EBO
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glm::mat4 trans1 = glm::mat4(1.0f);
		trans1 = glm::translate(trans1, glm::vec3(-0.5f, 0.5f, 0.0f));
		trans1 = glm::scale(trans1, glm::vec3((float)sin(glfwGetTime()), (float)sin(glfwGetTime()), 1.0f));
		unsigned int transformLoc1 = glGetUniformLocation(shader.ID, "transform");
		glUniformMatrix4fv(transformLoc1, 1, GL_FALSE, glm::value_ptr(trans1));
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		// swaps the front and back buffers of the specified window's double 
		glfwSwapBuffers(window);
		// checks if any input events are triggered, updates window state, and calls corresponding functions
		glfwPollEvents();
	}
#pragma endregion

	// cleans/deletes all of GLFW's resources once render loop is exited
	glfwTerminate();
	return 0;

}

// called each time the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


void processInput(GLFWwindow* window)
{
	// checks if user presses escape key and sets WindowShouldClose to true, making main renderer while loop end
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // glfwGetKey returns GLFW_RELEASE if not pressed
	{
		glfwSetWindowShouldClose(window, true);
	}
}
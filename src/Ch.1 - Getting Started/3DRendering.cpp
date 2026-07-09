#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader/shader.h>
#include <camera/camera.h>
#include<stb_image/stb_image.h>

#include <iostream>

// forward declarations of functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double dx, double dy);
void processInput(GLFWwindow* window);

// setting variables
const unsigned int SCR_WIDTH = 1920; // my screen dimensions
const unsigned int SCR_HEIGHT = 1200;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraForward = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
Camera camera(cameraPos, cameraForward, cameraUp);

// time variables
float deltaTime = 0.0f;
float lastTime = 0.0f;

int main()
{
#pragma region Initialization

	// initialize and configure the GLFW window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// creates a window object
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", glfwGetPrimaryMonitor(), NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// enables v-sync to sync fps to monitor. without it I had 2000+ fps, which made my GPU whine in fullscreen
	glfwSwapInterval(1);
	// sets own function as what to do when window size changes
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// enables cursor capture
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// registers mouse callback functions with GLFW
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// initialize GLAD to load OpenGL function pointers before calling any OpenGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// enables depth testing
	glEnable(GL_DEPTH_TEST);

#pragma endregion

#pragma region Shader

	Shader shader("Ch.1 - Getting Started/shaders/3DVertexShader.vs", "Ch.1 - Getting Started/shaders/3DFragmentShader.fs");

#pragma endregion

#pragma region Vertex

	// vertices of cube
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	// cube positions for multiple cubes
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	// creates IDs 
	unsigned int VAO, VBO;
	// generates one vertex array using the ID of the vertex array
	glGenVertexArrays(1, &VAO);
	// generates one vertex buffer using the ID of the vertex buffer
	glGenBuffers(1, &VBO);

	// binds vertex array
	glBindVertexArray(VAO);
	// binds the vertex buffer to the GL_ARRAY_BUFFER target
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// copies vertex data into the buffer's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// instructs how to read vertex attributes from the vertex buffer then enables it
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// texture
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

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

	// activates shaders
	shader.use();

	// sets texture uniforms in shader
	shader.setInt("texture1", 0);
	shader.setInt("texture2", 1);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		float currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// user input
		processInput(window);

		// rendering commands
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // state-setting function
		// clears color buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // state-using function

		// assigns texture to the fragment shader's uniform sampler2D 
		glActiveTexture(GL_TEXTURE0); // activates texture unit before binding
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1); // activates texture unit before binding
		glBindTexture(GL_TEXTURE_2D, texture2);

		// creates transformation matrices and sends them to the shader
		// view matrix
		glm::mat4 view;
		view = glm::lookAt(camera.pos, camera.pos + camera.forward, camera.up);
		shader.setMat4("view", view);

		// projection matrix
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camera.fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		shader.setMat4("projection", projection);

		// binds vertex array
		glBindVertexArray(VAO);
		for (unsigned int i = 0; i < 10; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(i * 10.0f), glm::vec3(0.5f, 1.0f, 0.0f));
			shader.setMat4("model", model);
			// draws when using VAO and VBO
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		

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

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}
	float dx = (float)xpos - lastX;
	float dy = lastY - (float)ypos;
	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.processMouseMovement(dx, dy);
}

void scroll_callback(GLFWwindow* window, double dx, double dy)
{
	camera.processMouseScroll(dx, dy);
}

void processInput(GLFWwindow* window)
{
	// checks if user presses escape key and sets WindowShouldClose to true, making main renderer while loop end
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // glfwGetKey returns GLFW_RELEASE if not pressed
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.moveCamera(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.moveCamera(BACK, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.moveCamera(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.moveCamera(RIGHT, deltaTime);
	}
}
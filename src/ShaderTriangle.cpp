#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shader/shader.h>

#include <iostream>

// forward declarations of functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// setting variables
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// vertex shader
const char* vertexShaderSource = "#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n" // "in" keyword declares the input vertex attributes
	"layout (location = 1) in vec3 aColor;\n"
	"out vec3 vertexColor;"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(aPos, 1.0);\n"
	"	vertexColor = aColor;"
	"}\0";

// fragment shader
const char* fragmentShaderSource = "#version 330 core\n"
	"out vec4 FragColor;"
	"in vec3 vertexColor;"
	//"uniform vec4 vertexColor;"
	"void main()\n"
	"{\n"
	"	FragColor = vec4(vertexColor, 1.0);"
	"}\n";

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
	Shader shader("vertexShader.vs", "fragmentShader.fs");
#pragma endregion

#pragma region Vertex
	// vertices of triangle, used with VAO and VBO
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, // bottom left
		 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // bottom right
		 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f  // top
	};
	//// vertices of rectangle, used with EBO
	//float vertices[] = {
	//	 0.5f,  0.5f, 0.0f,  // top right
	//	 0.5f, -0.5f, 0.0f,  // bottom right
	//	-0.5f, -0.5f, 0.0f,  // bottom left
	//	-0.5f,  0.5f, 0.0f   // top left 
	//};
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

	// instructs how to read vertex attributes from the vertex buffer
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	// enables vertex attribute
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// unbinds vertex buffer and vertex array 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
#pragma endregion

	// enables wireframe mode
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// user input
		processInput(window);

		// rendering commands
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // state-setting function
		glClear(GL_COLOR_BUFFER_BIT); // state-using function

		// activates shaders
		shader.use();

		float timeValue = glfwGetTime();
		float redValue = sin(2 * timeValue) / 2.0f + 0.5f;
		float greenValue = sin(2 * timeValue + 4.18879020479f) / 2.0f + 0.5f;
		float blueValue = sin(2 * timeValue + 2.09439510239f) / 2.0f + 0.5f;
		shader.setVec4("color", redValue, greenValue, blueValue, 1.0f);

		glBindVertexArray(VAO);
		// draws when using VAO and VBO
		glDrawArrays(GL_TRIANGLES, 0, 3);
		// draws when using EBO
		// glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// swaps the front and back buffers of the specified window's double 
		glfwSwapBuffers(window);
		// checks if any input events are triggered, updates window state, and calls corresponding functions
		glfwPollEvents();
	}

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
		glfwSetWindowShouldClose(window, true);
}
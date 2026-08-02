#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include<stb_image/stb_image.h>

#include <shader/shader.h>
#include <camera/camera.h>
#include <model/model.h>

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Renderer", glfwGetPrimaryMonitor(), NULL);
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
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#pragma endregion

#pragma region Light Object

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};

	unsigned int VBO, lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindVertexArray(lightVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

#pragma endregion

#pragma region Shader

	Shader shader("Ch.4-5 - Advanced Topics/shaders/light.vs", "Ch.4-5 - Advanced Topics/shaders/material.fs");
	Shader lightShader("Ch.4-5 - Advanced Topics/shaders/lightSource.vs", "Ch.4-5 - Advanced Topics/shaders/lightSource.fs");

#pragma endregion

#pragma region Texture

	stbi_set_flip_vertically_on_load(true);

#pragma endregion

#pragma region Model

	Model myModel("../resources/kiln/kiln.obj");

#pragma endregion

#pragma region Rendering

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		float currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// user input
		processInput(window);

		// rendering commands
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // state-setting function
		// clears color buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // state-using function

		// activates shaders
		shader.use();

		// creates transformation matrices and sends them to the shader
		// view matrix
		glm::mat4 view;
		view = glm::lookAt(camera.pos, camera.pos + camera.forward, camera.up);
		shader.setMat4("view", view);

		// projection matrix
		glm::mat4 projection;
		float near = 0.1f;
		float far = 100.0f;
		shader.setFloat("near", near);
		shader.setFloat("far", far);
		projection = glm::perspective(glm::radians(camera.fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, near, far);
		shader.setMat4("projection", projection);

		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.75f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.1f));	// it's a bit too big for our scene, so scale it down
		glm::mat3 normalMat = glm::transpose(inverse(model)); // necessary because transformations affect normal vectors differently than positions
		shader.setMat4("model", model);
		shader.setMat3("normalMat", normalMat);

		// lighting
		glm::vec3 lightColor = glm::vec3(1.0f);
		glm::vec3 lightPos = camera.pos;

		// light cube
		//float yPos = sin(2 * currentTime);
		//glm::vec3 lightPos2 = glm::vec3(0.0f, yPos, 2.0f);

		//lightShader.use();
		//lightShader.setMat4("projection", projection);
		//lightShader.setMat4("view", view);
		//glm::mat4 model2 = glm::mat4(1.0f);
		//model2 = glm::translate(model2, lightPos2);
		//model2 = glm::scale(model2, glm::vec3(0.1f));
		//lightShader.setMat4("model", model2);
		//glm::vec3 lightColor2 = glm::vec3(0.9f, 0.4f, 0.0f);
		//lightShader.setVec3("lightColor", lightColor2);
		//glBindVertexArray(lightVAO);
		//glDrawArrays(GL_TRIANGLES, 0, 36);

		shader.use();

		// directional light
		if (length(camera.pos - glm::vec3(0.0f, 0.0f, -35.0f)) < 28.0f) 
		{
			shader.setBool("directionalLight.enabled", true);
		}
		else
		{
			shader.setBool("directionalLight.enabled", false);
		}
		shader.setVec3("directionalLight.direction", glm::vec3(-0.1f, -1.0f, 0.2f)); // directional light
		shader.setVec3("directionalLight.ambient", lightColor * 0.1f);
		shader.setVec3("directionalLight.diffuse", lightColor * 0.5f);
		shader.setVec3("directionalLight.specular", lightColor * 0.5f);

		// point lights
		shader.setVec3("pointLights[0].position", lightPos);
		shader.setVec3("pointLights[0].ambient", lightColor * 0.1f);
		shader.setVec3("pointLights[0].diffuse", lightColor);
		shader.setVec3("pointLights[0].specular", lightColor);
		shader.setFloat("pointLights[0].constant", 1.0f);
		shader.setFloat("pointLights[0].linear", 0.22f);
		shader.setFloat("pointLights[0].quadratic", 0.20f);

		/*shader.setVec3("pointLights[1].position", lightPos2);
		shader.setVec3("pointLights[1].ambient", lightColor2 * 0.1f);
		shader.setVec3("pointLights[1].diffuse", lightColor2);
		shader.setVec3("pointLights[1].specular", lightColor2);
		shader.setFloat("pointLights[1].constant", 1.0f);
		shader.setFloat("pointLights[1].linear", 0.22f);
		shader.setFloat("pointLights[1].quadratic", 0.20f);*/

		shader.setVec3("viewPos", camera.pos);

		myModel.Draw(shader);

		// std::cout << camera.pos.x << " " << camera.pos.y << " " << camera.pos.z << std::endl;

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

	bool shiftHeld = false;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		shiftHeld = true;
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.moveCamera(FORWARD, shiftHeld, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.moveCamera(BACK, shiftHeld, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.moveCamera(LEFT, shiftHeld, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.moveCamera(RIGHT, shiftHeld, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		camera.moveCamera(DOWN, shiftHeld, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		camera.moveCamera(UP, shiftHeld, deltaTime);
	}
}
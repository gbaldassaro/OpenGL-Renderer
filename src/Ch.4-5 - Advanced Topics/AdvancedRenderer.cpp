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
void renderQuad();

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

bool showDepthBuffer = false;
bool bKeyPressed = false;

bool useNormalMap = false;
bool nKeyPressed = false;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 2.0f);
glm::vec3 cameraForward = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
Camera camera(cameraPos, cameraForward, cameraUp);

// time variables
float deltaTime = 0.0f;
float lastTime = 0.0f;

int main()
{
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
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader shader("Ch.4-5 - Advanced Topics/shaders/vertex.vs", "Ch.4-5 - Advanced Topics/shaders/fragment.fs");
	Shader depthShader("Ch.4-5 - Advanced Topics/shaders/shadowMapMaker.vs", "Ch.4-5 - Advanced Topics/shaders/shadowMapMaker.fs");
	Shader depthShaderDebug("Ch.4-5 - Advanced Topics/shaders/shadowMap.vs", "Ch.4-5 - Advanced Topics/shaders/shadowMap.fs");

	// shadow mapping
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	depthShaderDebug.use();
	depthShaderDebug.setInt("depthMap", 0);
	shader.use();
	shader.setInt("shadowMap", 10);

	// stbi_set_flip_vertically_on_load(true);

	Model myModel("[model path]");

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

		// render the depth map
		//glCullFace(GL_FRONT);
		//float nearPlane = 1.0f, farPlane = 7.5f;
		//glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, nearPlane, farPlane);
		glm::mat4 lightProjection;
		float nearPlane = 0.1f;
		float farPlane = 10.0f;
		lightProjection = glm::perspective(glm::radians(camera.fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, nearPlane, farPlane);
		//glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f, 3.0f, -15.0f),
		//								  glm::vec3(0.0f, 0.0f, -10.0f),
		//								  glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightView = glm::lookAt(camera.pos, camera.pos + camera.forward, camera.up);
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		depthShader.use();
		depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		glm::mat4 lightModel = glm::mat4(1.0f);
		lightModel = glm::translate(lightModel, glm::vec3(0.0f, 0.0f, 0.0f));
		lightModel = glm::scale(lightModel, glm::vec3(0.1f));	// it's a bit too big for our scene, so scale it down
		depthShader.setMat4("model", lightModel);

		myModel.Draw(depthShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);



		// render scene as normal with shadow mapping (using depth map)
		//glCullFace(GL_BACK);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

		shader.use();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		// creates transformation matrices and sends them to the shader
		// view matrix
		glm::mat4 view;
		view = glm::lookAt(camera.pos, camera.pos + camera.forward, camera.up);
		shader.setMat4("view", view);
		shader.setVec3("viewPos", camera.pos);

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
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f));	// it's a bit too big for our scene, so scale it down
		glm::mat3 normalMat = glm::transpose(inverse(model)); // necessary because transformations affect normal vectors differently than positions
		shader.setMat4("model", model);
		shader.setMat3("normalMat", normalMat);

		// lighting
		glm::vec3 lightColor = glm::vec3(1.0f);
		glm::vec3 lightPos = camera.pos;

		// directional light
		shader.setVec3("directionalLight.direction", glm::vec3(0.0f, 0.0f, -15.0f) - glm::vec3(0.0f, 17.0f, -35.0f)); // directional light
		shader.setVec3("directionalLight.center", glm::vec3(0.0f, 0.0f, -35.0f));
		shader.setFloat("directionalLight.innerRadius", 27.5f);
		shader.setFloat("directionalLight.outerRadius", 28.0f);
		shader.setVec3("directionalLight.ambient", lightColor * 0.05f);
		shader.setVec3("directionalLight.diffuse", lightColor * 0.15f);
		shader.setVec3("directionalLight.specular", lightColor * 0.15f);

		// point lights
		shader.setVec3("pointLights[0].position", lightPos);
		shader.setVec3("pointLights[0].ambient", lightColor * 0.1f);
		shader.setVec3("pointLights[0].diffuse", lightColor);
		shader.setVec3("pointLights[0].specular", lightColor);
		shader.setFloat("pointLights[0].constant", 1.0f);
		shader.setFloat("pointLights[0].linear", 0.7f); // range = 7
		shader.setFloat("pointLights[0].quadratic", 1.8f);

		glm::vec3 lightPos2 = glm::vec3(0.39f, 0.449f, -0.311f);
		glm::vec3 torchColor = glm::vec3(1.0f, 0.9f, 0.75f);
		shader.setVec3("pointLights[1].position", lightPos2);
		shader.setVec3("pointLights[1].ambient", torchColor * 0.1f);
		shader.setVec3("pointLights[1].diffuse", torchColor);
		shader.setVec3("pointLights[1].specular", torchColor);
		shader.setFloat("pointLights[1].constant", 1.0f);
		shader.setFloat("pointLights[1].linear", 0.9f); // range = 4
		shader.setFloat("pointLights[1].quadratic", 4.69f);

		glm::vec3 lightPos3 = glm::vec3(-0.465f, 0.449f, -0.311f);
		shader.setVec3("pointLights[2].position", lightPos3);
		shader.setVec3("pointLights[2].ambient", torchColor * 0.1f);
		shader.setVec3("pointLights[2].diffuse", torchColor);
		shader.setVec3("pointLights[2].specular", torchColor);
		shader.setFloat("pointLights[2].constant", 1.0f);
		shader.setFloat("pointLights[2].linear", 0.9f); // range = 4
		shader.setFloat("pointLights[2].quadratic", 4.69f);

		myModel.Draw(shader);

		depthShaderDebug.use();
		depthShaderDebug.setFloat("near_plane", nearPlane);
		depthShaderDebug.setFloat("far_plane", farPlane);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		if (showDepthBuffer) renderQuad();

		//std::cout << camera.pos.x << " " << camera.pos.y << " " << camera.pos.z << std::endl;

		// swaps the front and back buffers of the specified window's double buffer
		glfwSwapBuffers(window);
		// checks if any input events are triggered, updates window state, and calls corresponding functions
		glfwPollEvents();
	}

	// cleans/deletes all of GLFW's resources once render loop is exited
	glfwTerminate();
	return 0;

}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
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

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bKeyPressed)
	{
		showDepthBuffer = !showDepthBuffer;
		bKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
	{
		bKeyPressed = false;
	}

	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !nKeyPressed)
	{
		useNormalMap = !useNormalMap;
		nKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE)
	{
		nKeyPressed = false;
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
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		camera.moveCamera(DOWN, shiftHeld, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		camera.moveCamera(UP, shiftHeld, deltaTime);
	}
}
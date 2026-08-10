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

// number inputs
bool zeroKeyPressed = false;
bool oneKeyPressed = false;
bool twoKeyPressed = false;
bool threeKeyPressed = false;
bool fourKeyPressed = false;
bool fiveKeyPressed = false;
bool sixKeyPressed = false;
bool sevenKeyPressed = false;
bool eightKeyPressed = false;
bool nineKeyPressed = false;

// 0 = normal, 1 = invert,  2 = grayscale, 3 = sharpen, 4 = blur, 5 = edge-detection, 6 = bright, 7 = gaussian blur, 9 = shadow-depth
int postProcessingEffect;

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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	Shader mainShader("Ch.4-5 - Advanced Topics/shaders/vertex.vs", "Ch.4-5 - Advanced Topics/shaders/fragment.fs");
	Shader depthMapMakerShader("Ch.4-5 - Advanced Topics/shaders/depthMapMaker.vs", "Ch.4-5 - Advanced Topics/shaders/depthMapMaker.fs");
	Shader depthMapRendererShader("Ch.4-5 - Advanced Topics/shaders/framebufferRenderer.vs", "Ch.4-5 - Advanced Topics/shaders/depthMapRenderer.fs");
	Shader framebufferRendererShader("Ch.4-5 - Advanced Topics/shaders/framebufferRenderer.vs", "Ch.4-5 - Advanced Topics/shaders/framebufferRenderer.fs");
	
#pragma region Framebuffers

	/* 
	Framebuffers for off-screen rendering can have texture attachments or render buffer objects.
	Texture attachments are useful to directly sample data from a specific buffer.
	Renderbuffer objects have faster writes than textures but cannot be read from, so are useful if you never need to sample data.
	*/

	// main frame buffer
	unsigned int framebufferObject;
	glGenFramebuffers(1, &framebufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject);
	
	// create two color buffers: one for rendering scene as normal, and another for extracting bright portions
	unsigned int framebufferTextures[2];
	glGenTextures(2, framebufferTextures);
	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, framebufferTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, framebufferTextures[i], 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// depth and stencil render buffer
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	// GL_DEPTH24_STENCIL8 -> each 32 bit value contains 24 bits of depth info and 8 bits of stencil info
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// prepares to draw to multiple colorbuffers
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ping pong framebuffers (pair of framebuffers where we render and swap, a given number of times, the other framebuffer's 
	// color buffer into the current framebuffer's color buffer with an alternating shader effect)
	unsigned int pingpongfbo[2];
	glGenFramebuffers(2, pingpongfbo);

	unsigned int pingpongTextures[2];
	glGenTextures(2, pingpongTextures);
	for (int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongfbo[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH / 8, SCR_HEIGHT / 8, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongTextures[i], 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// shadow mapping
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	const unsigned int DEPTH_WIDTH = 4096, DEPTH_HEIGHT = 4096;

	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DEPTH_WIDTH, DEPTH_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion

	// stbi_set_flip_vertically_on_load(true);

	Model kiln("../resources/kiln/kiln_no_sky.obj");
	Model sky("../resources/kiln/sky.obj");
	Model clouds("../resources/kiln/clouds.obj");
	Model sun("../resources/kiln/sun.obj");

	// enables wireframe drawing
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		float currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// user input
		processInput(window);

#pragma region Depth Buffer

		// render the depth map
		glViewport(0, 0, DEPTH_WIDTH, DEPTH_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		//glCullFace(GL_FRONT);

		depthMapMakerShader.use();

		//float nearPlane = 1.0f, farPlane = 7.5f;
		//glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, nearPlane, farPlane);
		//glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f, 3.0f, -15.0f),
		//								  glm::vec3(0.0f, 0.0f, -10.0f),
		//								  glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 lightProjection;
		float nearPlane = 0.1f;
		float farPlane = 10.0f;
		lightProjection = glm::perspective(glm::radians(camera.fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, nearPlane, farPlane);
		glm::mat4 lightView = glm::lookAt(camera.pos, camera.pos + camera.forward, camera.up);

		glm::mat4 lightSpaceMatrix = lightProjection * lightView;
		depthMapMakerShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glm::mat4 lightModel = glm::mat4(1.0f);
		lightModel = glm::translate(lightModel, glm::vec3(0.0f, 0.0f, 0.0f));
		lightModel = glm::scale(lightModel, glm::vec3(0.1f));	// it's a bit too big for our scene, so scale it down
		depthMapMakerShader.setMat4("model", lightModel);

		kiln.Draw(depthMapMakerShader);

		//glCullFace(GL_BACK);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion

#pragma region Main Renderer Setup

		// render scene as normal with shadow mapping (using depth map)
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

		if (postProcessingEffect != 9)
		{
			// bind to framebuffer to draw scene to frame buffer texture attachment
			glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject);
			glEnable(GL_DEPTH_TEST);
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // state-setting function
		// clears color buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // state-using function

		mainShader.use();

		// creates transformation matrices and sends them to the shader
		// view matrix
		glm::mat4 view;
		view = glm::lookAt(camera.pos, camera.pos + camera.forward, camera.up);
		mainShader.setMat4("view", view);
		mainShader.setVec3("viewPos", camera.pos);

		// projection matrix
		glm::mat4 projection;
		float near = 0.1f;
		float far = 100.0f;
		mainShader.setFloat("near", near);
		mainShader.setFloat("far", far);
		projection = glm::perspective(glm::radians(camera.fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, near, far);
		mainShader.setMat4("projection", projection);

		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f));	// it's a bit too big for our scene, so scale it down
		glm::mat3 normalMat = glm::transpose(inverse(model)); // necessary because transformations affect normal vectors differently than positions
		mainShader.setMat4("model", model);
		mainShader.setMat3("normalMat", normalMat);

		// lighting
		glm::vec3 lightColor = glm::vec3(1.0f);
		glm::vec3 lightPos = camera.pos;

		// depth buffer and shadows
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		mainShader.setInt("depthMap", 0);

		mainShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

#pragma endregion

#pragma region Point Lights

		// point lights
		// player light
		mainShader.setVec3("pointLights[0].position", lightPos);
		mainShader.setVec3("pointLights[0].ambient", lightColor * 0.1f);
		mainShader.setVec3("pointLights[0].diffuse", lightColor);
		mainShader.setVec3("pointLights[0].specular", lightColor);
		mainShader.setFloat("pointLights[0].constant", 1.0f);
		mainShader.setFloat("pointLights[0].linear", 0.7f); // range = 7
		mainShader.setFloat("pointLights[0].quadratic", 1.8f);

		// various torches
		glm::vec3 torchColor = glm::vec3(1.0f, 0.9f, 0.75f);

		glm::vec3 lightPos1 = glm::vec3(0.39f, 0.449f, -0.311f);
		mainShader.setVec3("pointLights[1].position", lightPos1);
		mainShader.setVec3("pointLights[1].ambient", torchColor * 0.1f);
		mainShader.setVec3("pointLights[1].diffuse", torchColor);
		mainShader.setVec3("pointLights[1].specular", torchColor);
		mainShader.setFloat("pointLights[1].constant", 1.0f);
		mainShader.setFloat("pointLights[1].linear", 1.5f); // range = 3
		mainShader.setFloat("pointLights[1].quadratic", 8.33f);

		glm::vec3 lightPos2 = glm::vec3(-0.465f, 0.449f, -0.311f);
		mainShader.setVec3("pointLights[2].position", lightPos2);
		mainShader.setVec3("pointLights[2].ambient", torchColor * 0.1f);
		mainShader.setVec3("pointLights[2].diffuse", torchColor);
		mainShader.setVec3("pointLights[2].specular", torchColor);
		mainShader.setFloat("pointLights[2].constant", 1.0f);
		mainShader.setFloat("pointLights[2].linear", 1.5f); // range = 3
		mainShader.setFloat("pointLights[2].quadratic", 8.33f);

		glm::vec3 lightPos3 = glm::vec3(0.345f, 0.827f, -1.501f);
		mainShader.setVec3("pointLights[3].position", lightPos3);
		mainShader.setVec3("pointLights[3].ambient", torchColor * 0.1f);
		mainShader.setVec3("pointLights[3].diffuse", torchColor);
		mainShader.setVec3("pointLights[3].specular", torchColor);
		mainShader.setFloat("pointLights[3].constant", 1.0f);
		mainShader.setFloat("pointLights[3].linear", 4.5f); // range = 1
		mainShader.setFloat("pointLights[3].quadratic", 75.0f);

		glm::vec3 lightPos4 = glm::vec3(-0.371f, 0.827f, -1.501f);
		mainShader.setVec3("pointLights[4].position", lightPos4);
		mainShader.setVec3("pointLights[4].ambient", torchColor * 0.1f);
		mainShader.setVec3("pointLights[4].diffuse", torchColor);
		mainShader.setVec3("pointLights[4].specular", torchColor);
		mainShader.setFloat("pointLights[4].constant", 1.0f);
		mainShader.setFloat("pointLights[4].linear", 4.5f); // range = 1
		mainShader.setFloat("pointLights[4].quadratic", 75.0f);

		// fire lights
		glm::vec3 fireColor = glm::vec3(1.0f, 0.5f, 0.25f);
		
		glm::vec3 lightPos5 = glm::vec3(-1.644f, -0.248f, 0.381f);
		mainShader.setVec3("pointLights[5].position", lightPos5);
		mainShader.setVec3("pointLights[5].ambient", torchColor * 0.1f);
		mainShader.setVec3("pointLights[5].diffuse", torchColor);
		mainShader.setVec3("pointLights[5].specular", torchColor);
		mainShader.setFloat("pointLights[5].constant", 1.0f);
		mainShader.setFloat("pointLights[5].linear", 4.5f); // range = 1
		mainShader.setFloat("pointLights[5].quadratic", 75.0f);

		glm::vec3 lightPos6 = glm::vec3(1.532f, -0.248f, 0.438f);
		mainShader.setVec3("pointLights[6].position", lightPos6);
		mainShader.setVec3("pointLights[6].ambient", torchColor * 0.1f);
		mainShader.setVec3("pointLights[6].diffuse", torchColor);
		mainShader.setVec3("pointLights[6].specular", torchColor);
		mainShader.setFloat("pointLights[6].constant", 1.0f);
		mainShader.setFloat("pointLights[6].linear", 4.5f); // range = 1
		mainShader.setFloat("pointLights[6].quadratic", 75.0f);

		glm::vec3 lightPos7 = glm::vec3(-0.317f, 0.207f, -1.023f);
		mainShader.setVec3("pointLights[7].position", lightPos7);
		mainShader.setVec3("pointLights[7].ambient", torchColor * 0.1f);
		mainShader.setVec3("pointLights[7].diffuse", torchColor);
		mainShader.setVec3("pointLights[7].specular", torchColor);
		mainShader.setFloat("pointLights[7].constant", 1.0f);
		mainShader.setFloat("pointLights[7].linear", 4.5f); // range = 1
		mainShader.setFloat("pointLights[7].quadratic", 75.0f);

		glm::vec3 lightPos8 = glm::vec3(0.275f, 0.207f, -1.016f);
		mainShader.setVec3("pointLights[8].position", lightPos8);
		mainShader.setVec3("pointLights[8].ambient", torchColor * 0.1f);
		mainShader.setVec3("pointLights[8].diffuse", torchColor);
		mainShader.setVec3("pointLights[8].specular", torchColor);
		mainShader.setFloat("pointLights[8].constant", 1.0f);
		mainShader.setFloat("pointLights[8].linear", 4.5f); // range = 1
		mainShader.setFloat("pointLights[8].quadratic", 75.0f);

#pragma endregion

		if (postProcessingEffect != 9)
		{
			// binds the frame buffer texture to be drawn to
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, framebufferTextures[0]);
		}

		// draws sky separately to eliminate self-occlusion issues
		// directional light for sky
		mainShader.setVec3("directionalLights[0].direction", glm::vec3(0.0f, 1.0f, 0.0f)); // directional light
		mainShader.setVec3("directionalLights[0].center", glm::vec3(0.0f, 34.0f, -57.0f));
		mainShader.setFloat("directionalLights[0].innerRadius", 26.0f);
		mainShader.setFloat("directionalLights[0].outerRadius", 66.0f);
		mainShader.setVec3("directionalLights[0].ambient", lightColor * 0.10f);
		mainShader.setVec3("directionalLights[0].diffuse", lightColor * 0.20f);
		mainShader.setVec3("directionalLights[0].specular", lightColor * 0.20f);
		mainShader.setFloat("texOffset", 0.0f);

		glDisable(GL_DEPTH_TEST);
		sky.Draw(mainShader);
		//sun.Draw(mainShader);

		mainShader.setFloat("texOffset", currentTime * 0.01f);
		mainShader.setBool("useOpacityGradient", true);
		clouds.Draw(mainShader);
		mainShader.setFloat("texOffset", 0.0f);
		mainShader.setBool("useOpacityGradient", false);
		glEnable(GL_DEPTH_TEST);

		// directional light for kiln
		mainShader.setVec3("directionalLights[0].direction", glm::vec3(0.0f, 0.0f, -15.0f) - glm::vec3(0.0f, 17.0f, -35.0f)); // directional light
		mainShader.setVec3("directionalLights[0].center", glm::vec3(0.0f, 0.0f, -35.0f));
		mainShader.setFloat("directionalLights[0].innerRadius", 27.5f);
		mainShader.setFloat("directionalLights[0].outerRadius", 28.0f);
		mainShader.setVec3("directionalLights[0].ambient", lightColor * 0.10f);
		mainShader.setVec3("directionalLights[0].diffuse", lightColor * 0.10f);
		mainShader.setVec3("directionalLights[0].specular", lightColor * 0.10f);

		kiln.Draw(mainShader);

		// displays framebuffer post processing effect
		if (postProcessingEffect != 7 && postProcessingEffect != 9)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// clears color buffer and depth buffer
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			framebufferRendererShader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, framebufferTextures[0]);
			framebufferRendererShader.setInt("framebufferTexture", 0);
			framebufferRendererShader.setInt("postProcessingEffect", postProcessingEffect);
			glDisable(GL_DEPTH_TEST);
			renderQuad();
			glEnable(GL_DEPTH_TEST);
		}

		else if (postProcessingEffect == 7)
		{
			bool horizontal = true;
			bool firstIteration = true;
			int amount = 20;

			framebufferRendererShader.use();
			glActiveTexture(GL_TEXTURE0);
			framebufferRendererShader.setInt("framebufferTexture", 0);
			framebufferRendererShader.setInt("postProcessingEffect", postProcessingEffect);

			framebufferRendererShader.setBool("bloomReady", false);

			for (int i = 0; i < amount; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, pingpongfbo[horizontal]);
				glViewport(0, 0, SCR_WIDTH / 8, SCR_HEIGHT / 8);
				framebufferRendererShader.setBool("horizontal", horizontal);
				glBindTexture(GL_TEXTURE_2D, firstIteration ? framebufferTextures[1] : pingpongTextures[!horizontal]);
				renderQuad();
				horizontal = !horizontal;
				if (firstIteration) firstIteration = false;
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, framebufferTextures[0]);
			framebufferRendererShader.setInt("framebufferTexture", 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, pingpongTextures[!horizontal]);
			framebufferRendererShader.setInt("bloomTexture", 1);

			framebufferRendererShader.setBool("bloomReady", true);

			glDisable(GL_DEPTH_TEST);
			renderQuad();
			glEnable(GL_DEPTH_TEST);
		}

		// displays shadow map depth buffer
		else if (postProcessingEffect == 9)
		{
			depthMapRendererShader.use();
			depthMapRendererShader.setFloat("near_plane", nearPlane);
			depthMapRendererShader.setFloat("far_plane", farPlane);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			depthMapRendererShader.setInt("depthMap", 0);

			glDisable(GL_DEPTH_TEST);
			renderQuad();
			glEnable(GL_DEPTH_TEST);
		}
		

		// swaps the front and back buffers of the specified window's double buffer
		glfwSwapBuffers(window);
		// checks if any input events are triggered, updates window state, and calls corresponding functions
		glfwPollEvents();
	}

	// cleans/deletes all of GLFW's resources once render loop is exited
	glfwTerminate();
	return 0;

}


// renders single quad that covers view, used for displaying textures like depth buffer or for post-processing
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

#pragma region Number Inputs

	// no post-processing
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS && !zeroKeyPressed)
	{
		postProcessingEffect = 0;
		zeroKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_RELEASE)
	{
		zeroKeyPressed = false;
	}

	// invert
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !oneKeyPressed)
	{
		postProcessingEffect = 1;
		oneKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE)
	{
		oneKeyPressed = false;
	}

	// grayscale
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !twoKeyPressed)
	{
		postProcessingEffect = 2;
		twoKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE)
	{
		twoKeyPressed = false;
	}

	// sharpen
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !threeKeyPressed)
	{
		postProcessingEffect = 3;
		threeKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE)
	{
		threeKeyPressed = false;
	}

	// blur
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !fourKeyPressed)
	{
		postProcessingEffect = 4;
		fourKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE)
	{
		fourKeyPressed = false;
	}

	// edge-detection
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS && !fiveKeyPressed)
	{
		postProcessingEffect = 5;
		fiveKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE)
	{
		fiveKeyPressed = false;
	}

	// unused
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS && !sixKeyPressed)
	{
		postProcessingEffect = 6;
		sixKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE)
	{
		sixKeyPressed = false;
	}

	// unused
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS && !sevenKeyPressed)
	{
		postProcessingEffect = 7;
		sevenKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE)
	{
		sevenKeyPressed = false;
	}

	// unused
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS && !eightKeyPressed)
	{
		postProcessingEffect = 8;
		eightKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_RELEASE)
	{
		eightKeyPressed = false;
	}

	// shadow-depth
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS && !nineKeyPressed)
	{
		postProcessingEffect = 9;
		nineKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_RELEASE)
	{
		nineKeyPressed = false;
	}

#pragma endregion

#pragma region Movement Inputs

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
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		camera.moveCamera(DOWN, shiftHeld, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		camera.moveCamera(UP, shiftHeld, deltaTime);
	}

#pragma endregion

}
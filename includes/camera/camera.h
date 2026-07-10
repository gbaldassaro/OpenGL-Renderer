#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float sensitivity = 0.075f;
float speed = 30.0f;

float yaw = -90.0f;
float pitch = 0.0f;

enum moveDirection
{
	FORWARD,
	BACK,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera
{
public:

	// camera vectors
	glm::vec3 pos;
	glm::vec3 forward;
	glm::vec3 up;
	glm::vec3 right;

	float fov = 45.0f;

	// constructor to build the camera
	Camera(glm::vec3 cameraPos, glm::vec3 cameraForward, glm::vec3 cameraUp)
	{
		pos = cameraPos;
		forward = cameraForward;
		up = cameraUp;

		updateCameraDirection();
	}

	void moveCamera(moveDirection direction, float deltaTime)
	{
		float velocity = speed * deltaTime;
		if (direction == FORWARD)
		{
			pos += velocity * forward;
		}
		if (direction == BACK)
		{
			pos -= velocity * forward;
		}
		if (direction == LEFT)
		{
			pos -= velocity * right;
		}
		if (direction == RIGHT)
		{
			pos += velocity * right;
		}
		if (direction == UP)
		{
			pos += velocity * up;
		}
		if (direction == DOWN)
		{
			pos -= velocity * up;
		}
	}

	void processMouseMovement(float dx, float dy)
	{
		dx *= sensitivity;
		dy *= sensitivity;

		yaw += dx;
		pitch += dy;

		if (pitch > 89.0f)
		{
			pitch = 89.0f;
		}
		if (pitch < -89.0f)
		{
			pitch = -89.0f;
		}

		updateCameraDirection();
	}

	void processMouseScroll(double dx, double dy)
	{
		fov -= (float)dy;
		if (fov < 1.0f)
		{
			fov = 1.0f;
		}
		if (fov > 80.0f)
		{
			fov = 80.0f;
		}
	}

private:

	void updateCameraDirection()
	{
		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		forward = glm::normalize(direction);

		right = glm::normalize(glm::cross(forward, up));
	}
};

#endif

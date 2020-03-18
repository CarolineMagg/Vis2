#pragma once
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include "CameraMovement.h"
#include "../render/FrustumCull.h"
#include <memory>

class Camera {
public:
	Camera(float fov, float aspectRatio, float near, float far);
	Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up);
	~Camera();

	FrustumCull ftc;

	void frameUpdate(float deltaT);
	void setPosition(float x, float y, float z);
	void setPosition(const glm::vec3& pos);
	void setLookAt(const glm::vec3& lookAt);

	const glm::vec3& getPosition() const;	
	const glm::vec3& getFront() const;	
	const glm::vec3& getUp() const;
	const glm::mat4& getViewMatrix() const;
	const glm::mat4& getProjection() const;
	void processKeyInput(GLFWwindow *window, float frameDeltaTime);
	void processMouseInput(GLFWwindow* window, double xpos, double ypos, float frameDeltaTime);

	void setSpeed(float newSpeed);
	float getSpeed();
	void resetCamera(glm::vec3 pos, glm::vec3 front, bool isLocked);
	
	// Frustum Plane Distances
	GLfloat nearPlane = 0.1f;
	GLfloat farPlane = 10.f;
	GLfloat Zoom;

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;

	bool locked = false;

private:	
	glm::mat4 viewMatrix;
	glm::mat4 projection;

	std::unique_ptr<CameraMovement> cameraMovement;

	float speed = 12.00f;
	float mouseSensitivity = 0.05f;

	double lastMouseX = 0;
	double lastMouseY = 0;
	bool initMouse = true;

	float pitch = 0.0f;
	float yaw = -90.0f;

	void calculateFront();

	void init();
};
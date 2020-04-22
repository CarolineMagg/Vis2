#include <glm\gtc\matrix_transform.hpp>
#include <iostream>
#include "Camera.h"
#include "Constants.h"
#include "../shading/Shader.h"

Camera::Camera(float fov, float aspectRatio, float near, float far) {	
	this->position = glm::vec3(0.0f, 0.0f, 0.0f);
	this->front = glm::vec3(0.65f, -0.4f, -0.65f);
	//this->position = glm::vec3(0.0f, 2.0f, 0.0f);
	//this->front = glm::vec3(0.0f, 0.0f, -1.0f);
	this->up = glm::vec3(0.0f, 1.0f, 0.0f);
	projection = glm::perspective(glm::radians(fov), aspectRatio, near, far);	

	nearPlane = near;
	farPlane = far;
		
	init();
}

Camera::Camera(glm::vec3 aPosition, glm::vec3 aFront, glm::vec3 aUp) {
	position = aPosition;
	front = aFront;
	up = aUp;
	
	init();
}

Camera::~Camera() {
}


void Camera::init() {
	//ftc.setCamInternals(fo)
}

void Camera::frameUpdate(float deltaT) {
	if (!locked) {
		calculateFront();
		viewMatrix = glm::lookAt(position, position + front, up);				
	}	
}

void Camera::setPosition(float x, float y, float z) {
	setPosition(glm::vec3(x, y, z));
}

void Camera::setPosition(const glm::vec3& aPosition) {
	position = aPosition;
}

void Camera::setLookAt(const glm::vec3 & lookAt)
{
	front = glm::normalize(lookAt-position);

}

const glm::vec3& Camera::getPosition() const {	
	return position;
}

const glm::vec3& Camera::getFront() const {
	return front;
}

const glm::vec3& Camera::getUp() const {
	return up;
}

void Camera::setSpeed(float newSpeed) {
	speed = newSpeed;
}

float Camera::getSpeed() {
	return speed;
}

const glm::mat4& Camera::getViewMatrix() const {
	return viewMatrix;
}

const glm::mat4& Camera::getProjection() const {
	return projection;
}

void Camera::resetCamera(glm::vec3 pos, glm::vec3 fro, bool isLocked) {
	locked = isLocked;
	position = pos;
	if (locked)
		front = fro;
}

void Camera::calculateFront() {
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	front = glm::normalize(front);
}

void Camera::processKeyInput(GLFWwindow *window, float frameDeltaTime) {
	if (locked) return;

	float sp = speed * frameDeltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		if (USE_FREE_MOVE_CAMERA) {
			position += sp * front;
		} else {
			position += sp * glm::vec3(front.x, 0, front.z);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		if (USE_FREE_MOVE_CAMERA) {
			position -= sp * front;
		} else {
			position -= sp * glm::vec3(front.x, 0, front.z);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= glm::normalize(glm::cross(front, up)) * sp;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += glm::normalize(glm::cross(front, up)) * sp;
	}
}

void Camera::processMouseInput(GLFWwindow* window, double xpos, double ypos, float frameDeltaTime) {
	if (initMouse) {
		lastMouseX = xpos;
		lastMouseY = ypos;
		initMouse = false;
	}

	double offsetX = (xpos - lastMouseX) * mouseSensitivity;
	double offsetY = (lastMouseY - ypos) * mouseSensitivity;

	lastMouseX = xpos;
	lastMouseY = ypos;

	if (locked) return;

	yaw += (float)offsetX;
	pitch += (float)offsetY;

	if (pitch > 85.0)  // add this as option
		pitch = 85.0;
	if (pitch < -69.0)
		pitch = -69.0;
}

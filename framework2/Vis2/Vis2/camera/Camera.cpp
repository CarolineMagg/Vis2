#include <glm\gtc\matrix_transform.hpp>
#include <iostream>
#include "Camera.h"
#include "Constants.h"
#include "../shading/Shader.h"

Camera::Camera(float fov, float aspectRatio, float near, float far) {	
	this->position = glm::vec3(0.0f, 0.0f, 2.0f);
	this->front = glm::vec3(0.0f, 0.0f, 1.0f);
	this->up = glm::vec3(0.0, 1.0, 0.0);
	this->distance = 2.0f;
	this->pitch = -25.0f;
	this->yaw = +120.0f;

	projection = glm::perspective(glm::radians(fov), aspectRatio, near, far);
	nearPlane = near;
	farPlane = far;
}

Camera::~Camera() {
}

void Camera::frameUpdate(float deltaT) {
	calculateFront();
	position = - front * glm::vec3(distance);
	viewMatrix = glm::lookAt(position, glm::vec3(0,0,0), up);	

}

void Camera::calculateFront() {
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	front = glm::normalize(front);

}

void Camera::processKeyInput(GLFWwindow *window, float frameDeltaTime) {

	float sp = speed * frameDeltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		pitch -= 1.0 * sp;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		pitch += 1.0 * sp;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		yaw += 1.0 * sp;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		yaw -= 1.0 * sp;
	}

	if (pitch > 80.0)
		pitch = 80.0;
	if (pitch < -80.0)
		pitch = -80.0;
	
}

void Camera::autoRotate(float frameDeltaTime) {
	float sp = speed * frameDeltaTime;
	yaw += 1.0 * sp;
}

void Camera::processScroll(double yoffset) {

	distance -= yoffset * 0.1;

	if (distance <= 0.5f) {
		distance = 0.5f;
	}

}

void Camera::resetCamera() {
	this->position = glm::vec3(0.0f, 0.0f, 2.0f);
	this->front = glm::vec3(0.0f, 0.0f, 1.0f);
	this->up = glm::vec3(0.0, 1.0, 0.0);
	this->distance = 2.0f;
	this->pitch = -25.0f;
	this->yaw = +120.0f;
}

void Camera::setPosition(float x, float y, float z) {
	setPosition(glm::vec3(x, y, z));
}

void Camera::setPosition(const glm::vec3& aPosition) {
	position = aPosition;
}

void Camera::setLookAt(const glm::vec3 & lookAt)
{
	front = glm::normalize(lookAt - position);
}

void Camera::setFront(const glm::vec3 & newFront) {
	front = glm::normalize(newFront);
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

const glm::mat4& Camera::getViewMatrix() const {
	return viewMatrix;
}

const glm::mat4& Camera::getProjection() const {
	return projection;
}

const float & Camera::getYaw() const
{
	return yaw;
}

const float & Camera::getPitch() const
{
	return pitch;
}
#pragma once
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <memory>

class Camera {
public:
	Camera(float fov, float aspectRatio, float near, float far);
	~Camera();
		
	void frameUpdate(float deltaT);
	void setPosition(float x, float y, float z);
	void setPosition(const glm::vec3& pos);
	void setLookAt(const glm::vec3& lookAt);
	void setFront(const glm::vec3 & newFront);

	const glm::vec3& getPosition() const;	
	const glm::vec3& getFront() const;	
	const glm::vec3& getUp() const;
	const glm::mat4& getViewMatrix() const;
	const glm::mat4& getProjection() const;
	const float& getYaw() const;
	const float& getPitch() const;

	void processKeyInput(GLFWwindow *window, float frameDeltaTime);
	void processScroll(double yoffset);
	void autoRotate(float frameDeltaTime);
	void resetCamera();
	
	// Frustum Plane Distances
	GLfloat nearPlane = 0.1f;
	GLfloat farPlane = 40.f;

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;

	float speed = 5.00f;

private:	
	glm::mat4 viewMatrix;
	glm::mat4 projection;
	
	float pitch;
	float yaw;
	float distance;
	
	void calculateFront();

};
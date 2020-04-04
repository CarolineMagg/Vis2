#pragma once
#include <glm/glm.hpp>
#include <vector>

struct PathPoint
{
	glm::vec3 position;
	glm::vec3 lookAt;
	float timePoint;
	PathPoint(glm::vec3 position, glm::vec3 lookat, float timePoint)
		:position(position),lookAt(lookat), timePoint(timePoint) {};
	PathPoint():position(glm::vec3(0.0f)), lookAt(glm::vec3(0.0f)), timePoint(0.0f){}
};
class CameraMovement
{
private:
	std::vector<PathPoint> cameraPath;
	float currentTime;
	int currentPoint;

	glm::vec3 position = glm::vec3(0.0f,0.0f,0.0f);
	glm::vec3 lookAt = glm::vec3(0.0f, 0.0f, -1.0f);

	void extendToMinSize();
public:
	CameraMovement();
	~CameraMovement();
	void update(float deltaT);

	glm::vec3 getPosition();
	glm::vec3 getLookAt();
};


#include "CameraMovement.h"
#include <glm/gtx/spline.hpp>

#include <iostream>

void CameraMovement::extendToMinSize()
{
	if (cameraPath.size() < 4) 
	{
		int size = cameraPath.size();
		for (int i = size; i < 4; ++i)
		{
			cameraPath.push_back(cameraPath[i]);
		}
	}
}

CameraMovement::CameraMovement()
{
	//Maybe read from file
	//cameraPath.push_back(PathProp(glm::vec3(-17.5f, 21.0f, 32.0f), glm::vec3(-17.5f, 21.0f, 32.0f), glm::vec3(-17.5f, 21.0f, 32.0f), glm::vec3(-17.5f, 21.0f, 32.0f),
	//	glm::vec3(4.0f, 0.0f, -36.2f), glm::vec3(4.0f, 0.0f, -36.2f), glm::vec3(4.0f, 0.0f, -36.2f), glm::vec3(4.0f, 0.0f, -36.2f), 2.f));
	/*
	currentPoint = 0;
	currentTime = 0.0f;
	cameraPath.push_back(PathPoint(glm::vec3(-7.63414, 3.23881, -16.278), glm::vec3(-5.47063, 1.66409, 10.7106), 0.0f));
	cameraPath.push_back(PathPoint(glm::vec3(-6.63414, 3.23881, -16.278), glm::vec3(-5.47063, 1.66409, 10.7106), 8.0f));
	cameraPath.push_back(PathPoint(glm::vec3(3.41269, 12.0936, 4.23739), glm::vec3(-5.47063, 1.66409, 10.7106), 13.0f));
	cameraPath.push_back(PathPoint(glm::vec3(8.02888, 2.93328, -15.4621), glm::vec3(-5.47063, 1.66409, 10.7106), 19.0f));
	cameraPath.push_back(PathPoint(glm::vec3(-8.44531, 2.28574, -15.5889), glm::vec3(-5.47063, 1.66409, 10.7106), 27.0f));
	cameraPath.push_back(PathPoint(glm::vec3(-3.41136, 5.41736, -7.10077), glm::vec3(8.10482, 1.2406, -1.04137), 34.0f));
	cameraPath.push_back(PathPoint(glm::vec3(5.17289, 8.63793, 2.36446), glm::vec3(8.10482, 1.2406, -1.04137), 41.0f));
	cameraPath.push_back(PathPoint(glm::vec3(1.65275, 10.3337, 10.2752), glm::vec3(-5.47063, 1.66409, 10.7106), 46.0f));	
	cameraPath.push_back(PathPoint(glm::vec3(-6.62411, 3.04661, 14.1496), glm::vec3(-5.10664, 0.646004, 11.2775), 53.0f));
	cameraPath.push_back(PathPoint(glm::vec3(-8.91817, 3.40773, 8.80646), glm::vec3(-5.10664, 0.646004, 11.2775), 60.0f));
	cameraPath.push_back(PathPoint(glm::vec3(-6.58315, 8.19974, -3.499), glm::vec3(3.84506, 2.05215, -1.36958), 69.0f));
	cameraPath.push_back(PathPoint(glm::vec3(3.71088, 0.88015, -8.08549), glm::vec3(9.61775, 0.318206, -15.1873), 85.0f));
	cameraPath.push_back(PathPoint(glm::vec3(-0.759698, 0.96414, -11.8792), glm::vec3(-0.656108, 1.05738, -15.8346), 87.0f));
	cameraPath.push_back(PathPoint(glm::vec3(-8.1087, 0.333845, 3.19158), glm::vec3(-10.1841, 0.292815, -2.10319), 90.0f));
	cameraPath.push_back(PathPoint(glm::vec3(-7.80269, 5.82485, 14.9532), glm::vec3(-3.84778, 4.67186, 9.0914), 94.0f));
	cameraPath.push_back(PathPoint(glm::vec3(8.27185, 1.93387, 5.49416), glm::vec3(7.24835, 1.04258, 3.43702), 105.0f));
	cameraPath.push_back(PathPoint(glm::vec3(5.60984, 12.4359, -10.2629), glm::vec3(3.59839, 9.46832, -6.45386), 114.0f));
	cameraPath.push_back(PathPoint(glm::vec3(-7.55806, 13.4359, -14.2489), glm::vec3(-4.92072, 10.129, -9.74077), 125.0f));
	*/
	
	//position = cameraPath[0].position;
	//lookAt = cameraPath[0].lookAt;
	//extendToMinSize();

}


CameraMovement::~CameraMovement()
{
}

void CameraMovement::update(float deltaT)
{
	currentTime += deltaT;
	//We need following points for interpolation to interpolate between thisPoint and nextPoint
	PathPoint *interpolatingBefore, *thisPoint, *nextPoint, *interpolatingAfter;
	if (currentPoint < cameraPath.size() - 2)
	{
		//normal
		//Get Start and Endpoint
		thisPoint = &cameraPath[currentPoint];
		nextPoint = &cameraPath[currentPoint+1];
		if (currentTime > nextPoint->timePoint)
		{
			++currentPoint;
			thisPoint = nextPoint;
			nextPoint = &cameraPath[currentPoint + 1];
		}
		//Calculating interpolating points for smoothness
		//before
		if (currentPoint == 0) 
		{
			interpolatingBefore = thisPoint;
		}
		else 
		{
			interpolatingBefore = &cameraPath[currentPoint - 1];
		}
		//after
		if (currentPoint == cameraPath.size() - 2)
		{
			interpolatingAfter = nextPoint;
		}
		else
		{
			interpolatingAfter = &cameraPath[currentPoint + 2];
		}
	}
	else if (currentPoint == cameraPath.size() - 2)
	{
		//last part
		thisPoint = &cameraPath[currentPoint];
		nextPoint = &cameraPath[currentPoint + 1];
		if (currentTime > nextPoint->timePoint)
		{
			//CameraPath has finished
			++currentPoint;
			position = nextPoint->position;
			lookAt = nextPoint->lookAt;
			return;
		}
		interpolatingBefore = &cameraPath[currentPoint - 1];
		interpolatingAfter = nextPoint;
	}
	else
	{
		//finished
		return;
	}
	//Calculate interpolation factor (between 0 and 1)
	float duration = nextPoint->timePoint - thisPoint->timePoint;

	float interpolator = (currentTime-thisPoint->timePoint) / duration;
	position = glm::catmullRom(interpolatingBefore->position, thisPoint->position, nextPoint->position, interpolatingAfter->position, interpolator);
	lookAt = glm::catmullRom(interpolatingBefore->lookAt, thisPoint->lookAt, nextPoint->lookAt, interpolatingAfter->lookAt, interpolator);
}

glm::vec3 CameraMovement::getPosition()
{
	return position;
}

glm::vec3 CameraMovement::getLookAt()
{
	return lookAt;
}

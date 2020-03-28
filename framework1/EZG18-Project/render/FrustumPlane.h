// inspired by http://www.lighthouse3d.com/tutorials/view-frustum-culling/
#pragma once
#include "glm/glm.hpp"

class FrustumPlane {

public:

	glm::vec3 normal, point;
	float d;

	FrustumPlane(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3);
	FrustumPlane(void);	

	void set3Points(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3);
	//void setNormalAndPoint(const glm::vec3 &normal, const glm::vec3 &point);
	//void setCoefficients(float a, float b, float c, float d);
	float distance(const glm::vec3 &p) const;
};



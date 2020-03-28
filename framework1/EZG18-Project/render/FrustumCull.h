// inspired by http://www.lighthouse3d.com/tutorials/view-frustum-culling/
#pragma once
#include "glm/glm.hpp"
#include "FrustumPlane.h"

class FrustumCull {

public:

	enum { OUTSIDE, INTERSECT, INSIDE };

	FrustumPlane pl[6];

	glm::vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
	float nearD, farD, ratio, angle, tang;
	float nw, nh, fw, fh;
	
	void setCamInternals(float angle, float ratio, float nearD, float farD);
	void setCamDef(const glm::vec3 &p, const glm::vec3 &l, const glm::vec3 &u);
	int pointInFrustum(const glm::vec3 &p) const;
	int sphereInFrustum(const glm::vec3 &p, float raio) const;
	//int boxInFrustum(const FrustumAABox &b) const;

	//void drawPoints();
	//void drawLines();
	//void drawPlanes();
	//void drawNormals();

private:

	enum {
		TOP = 0,
		BOTTOM,
		LEFT,
		RIGHT,
		NEARP,
		FARP
	};

};
// inspired by http://www.lighthouse3d.com/tutorials/view-frustum-culling/
#include "FrustumPlane.h"

FrustumPlane::FrustumPlane(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3) {

	set3Points(v1, v2, v3);
}


FrustumPlane::FrustumPlane() {}


void FrustumPlane::set3Points(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3) {

	glm::vec3 aux1, aux2;

	aux1 = v1 - v2;
	aux2 = v3 - v2;

	normal = glm::normalize(glm::cross(aux2, aux1));

	point = v2;
	d = -(glm::dot(normal, point));
}


float FrustumPlane::distance(const glm::vec3 &p) const {

	return (d + glm::dot(normal, p));
}

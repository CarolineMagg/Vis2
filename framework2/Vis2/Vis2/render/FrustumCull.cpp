// inspired by http://www.lighthouse3d.com/tutorials/view-frustum-culling/
#include "FrustumCull.h"

void FrustumCull::setCamInternals(float angle, float ratio, float nearD, float farD) {

	this->ratio = ratio;
	this->angle = angle;
	this->nearD = nearD;
	this->farD = farD;

	tang = (float)tan(glm::radians(angle) * 0.5);
	nh = nearD * tang;
	nw = nh * ratio;
	fh = farD * tang;
	fw = fh * ratio;
}


void FrustumCull::setCamDef(const glm::vec3 &p, const glm::vec3 &l, const glm::vec3 &u) {

	glm::vec3 dir, nc, fc, X, Y, Z;
		
	Z = glm::normalize(p - l);
	X = glm::normalize(glm::cross(u, Z));

	Y = glm::cross(Z, X);

	nc = p - Z * nearD;
	fc = p - Z * farD;

	glm::vec3 Xnw = X * nw;
	glm::vec3 Xfw = X * fw;
	glm::vec3 Ynh = Y * nh;
	glm::vec3 Yfh = Y * fh;

	ntl = nc + Ynh - Xnw;
	ntr = nc + Ynh + Xnw;
	nbl = nc - Ynh - Xnw;
	nbr = nc - Ynh + Xnw;

	ftl = fc + Yfh - Xfw;
	ftr = fc + Yfh + Xfw;
	fbl = fc - Yfh - Xfw;
	fbr = fc - Yfh + Xfw;

	pl[TOP].set3Points(ntr, ntl, ftl);
	pl[BOTTOM].set3Points(nbl, nbr, fbr);
	pl[LEFT].set3Points(ntl, nbl, fbl);
	pl[RIGHT].set3Points(nbr, ntr, fbr);
	pl[NEARP].set3Points(ntl, ntr, nbr);
	pl[FARP].set3Points(ftr, ftl, fbl);
}


int FrustumCull::pointInFrustum(const glm::vec3 &p) const {

	int result = INSIDE;
	for (int i = 0; i < 6; i++) {

		if (pl[i].distance(p) < 0)
			return OUTSIDE;
	}
	return result;
}


int FrustumCull::sphereInFrustum(const glm::vec3 &p, float raio) const {
		
	int result = INSIDE;
	float distance;

	for(int i=0; i < 6; i++) {
		distance = pl[i].distance(p);
		if (distance < -raio)
			return OUTSIDE;
		else if (distance < raio)
			result =  INTERSECT;
	}
	return(result);
}





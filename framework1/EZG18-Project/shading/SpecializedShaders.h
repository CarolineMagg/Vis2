#pragma once
#include <gl3w.h>
#include "Shader.h"
#include "../lights/Light.h"
#include "../camera/Camera.h"

class VolumetricLightShader : public Shader {
public:
	VolumetricLightShader(LightManager& lightManager);
	void setupRender(LightManager& lightManager, const Camera& camera, unsigned int sceneDepthTexture);
private:
	unsigned int lightUniformsSubscriptionID;	
};

class MainShader : public Shader {
public:
	MainShader(LightManager& lightManager);
	MainShader() = delete;
	void setupRender(LightManager& lightManager, const Camera& camera);
private:
	unsigned int lightUniformsSubscriptionID;
};
#include "SpecializedShaders.h"

VolumetricLightShader::VolumetricLightShader(LightManager& lightManager) : Shader("vol_vs.txt", "vol_fs.txt") {
	lightUniformsSubscriptionID = lightManager.subscribeToLightUniforms();
}

void VolumetricLightShader::setupRender(LightManager& lightManager, const Camera& camera, unsigned int sceneDepthTexture) {
	use();
	//update light uniforms
	const std::vector<SpotLight>& spotLights = lightManager.getSpotLights();
	unsigned int spotLightSize = spotLights.size();
	if (lightManager.isUniformDirty(lightUniformsSubscriptionID)) {		
		for (unsigned int i = 0; i < spotLightSize; i++) {
			std::string si = std::to_string(i);
			const LightProperties &properties = spotLights[i].getProperties();
			setUniform("spotLights[" + si + "].direction", properties.direction);
			setUniform("spotLights[" + si + "].cutOff", glm::cos(glm::radians(properties.cutOff)));
			setUniform("spotLights[" + si + "].outerCutOff", glm::cos(glm::radians(properties.outerCutOff)));

			setUniform("spotLights[" + si + "].energy", properties.energy);
			setUniform("spotLights[" + si + "].position", properties.position);

			setUniform("spotLights[" + si + "].constant", properties.constant);
			setUniform("spotLights[" + si + "].linear", properties.linear);
			setUniform("spotLights[" + si + "].quadratic", properties.quadratic);

			setUniform("spotLights[" + si + "].bias", properties.bias);			
			
			setUniform("spotLightsDepthMaps[" + si + "]", i);
			spotLights[i].updateMatricesUniforms(*this);

			setUniform("spotLightsVolume[" + si + "].hasVolume", properties.vp.hasVolume);
			setUniform("spotLightsVolume[" + si + "].phi", properties.vp.phi);
			setUniform("spotLightsVolume[" + si + "].tau", properties.vp.tau);
		}
		lightManager.onUniformUpdate(lightUniformsSubscriptionID);
	}

	for (unsigned int i = 0; i < spotLightSize; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, spotLights[i].depthMap);
		setUniform("spotLights[" + std::to_string(i) + "].isActive", spotLights[i].isActive);
	}

	glActiveTexture(GL_TEXTURE0 + spotLightSize);
	setUniform("sceneDepth", spotLightSize);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, sceneDepthTexture);	

	setUniform("inverseView", glm::inverse(camera.getViewMatrix()));
	setUniform("inverseProjection", glm::inverse(camera.getProjection()));
	setUniform("viewPos", camera.getPosition());
}
#include "SpecializedShaders.h"

MainShader::MainShader(LightManager& lightManager) : Shader("basicShader_vs.txt", "basicShader_fs.txt") {
	lightUniformsSubscriptionID = lightManager.subscribeToLightUniforms();
}
int tt = 0;
void MainShader::setupRender(LightManager& lightManager, const Camera& camera) {
	use();
	//update light uniforms
	const std::vector<SpotLight>& spotLights = lightManager.getSpotLights();
	unsigned int spotLightSize = spotLights.size();
	const std::vector<PointLight>& pointLights = lightManager.getPointLights();
	unsigned int pointLightSize = pointLights.size();
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
			setUniform("lightSpaceMatrix[" + si + "]", spotLights[i].getLightSpaceMatrix());
		}

		for (unsigned int i = 0; i < pointLightSize; i++) {
			std::string si = std::to_string(i);
			const LightProperties &properties = pointLights[i].getProperties();
			
			setUniform("pointLights[" + si + "].energy", properties.energy);
			setUniform("pointLights[" + si + "].position", properties.position);

			setUniform("pointLights[" + si + "].constant", properties.constant);
			setUniform("pointLights[" + si + "].linear", properties.linear);
			setUniform("pointLights[" + si + "].quadratic", properties.quadratic);

			setUniform("pointLights[" + si + "].bias", properties.bias);
			
						
			setUniform("pointLightsDepthMaps[" + si + "]", i + spotLightSize);			
		}
		setUniform("far_plane", Light::FAR_PLANE);
		lightManager.onUniformUpdate(lightUniformsSubscriptionID);
	}

	for (unsigned int i = 0; i < spotLightSize; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, spotLights[i].depthMap);
		setUniform("spotLights[" + std::to_string(i) + "].isActive", spotLights[i].isActive);
	}
	for (unsigned int i = 0; i < pointLightSize; i++) {
		glActiveTexture(GL_TEXTURE0 + i + spotLightSize);
		glBindTexture(GL_TEXTURE_CUBE_MAP, pointLights[i].depthMap);
		setUniform("pointLights[" + std::to_string(i) + "].isActive", pointLights[i].isActive);
	}

	const glm::mat4 projectionViewMatrix = camera.getProjection() * camera.getViewMatrix();
	setUniform("viewProjection", projectionViewMatrix);
	setUniform("viewPos", camera.getPosition());
}
#include <gl3w.h>
#include "Light.h"
#include "SLParticleSystem.h"


const float Light::FAR_PLANE = 100.0f;
Light::Light(const LightProperties& properties, LightManager& lightManager) : properties(properties), lightManager(lightManager) {}

LightManager::LightManager() :
	depthShader{"depth_vs.txt", "depth_fs.txt"},
	omniDepthShader{ "omni_depth_vs.txt", "omni_depth_fs.txt", "omni_depth_gs.txt" },
	particleRender{ "partRender_vs.txt", "partRender_fs.txt", "partRender_gs.txt" }
{	
	//depth frame buffer
	glGenFramebuffers(1, &depthBufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthBufferFBO);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
	std::vector<GLchar> varys = { 'T', 'P', 'V', 'A' };	
	particleUpdate = std::make_shared<Shader>("ps_update_vs.txt", "ps_update_fs.txt", "ps_update_gs.txt", true, &varys);
}

int LightManager::addSpotLight(const LightProperties& properties) {	
	spotLights.push_back(std::move(SpotLight{ properties, *this }));

	setAllUniformsDirty();
	depthTextureCount++;
	return spotLights.size() - 1;
}

int LightManager::addSpotLight(glm::vec3 position, glm::vec3 energy, glm::vec3 direction, float cutOff, float outerCutoff) {
	LightProperties lp{ position, direction, energy };
	lp.cutOff = cutOff;
	lp.outerCutOff = outerCutoff;
	return addSpotLight(lp);
}
int LightManager::addPointLight(const LightProperties& properties) {
	pointLights.push_back(std::move(PointLight{ properties, *this }));

	setAllUniformsDirty();	
	depthTextureCount++;
	return pointLights.size() - 1;
}

int LightManager::addPointLight(glm::vec3 position, glm::vec3 energy) {
	LightProperties lp{ position, energy };
	return addPointLight(lp);
}

void LightManager::renderDepthMaps(const std::unordered_map<std::string, std::shared_ptr<SceneObject>> &models) {
	//glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthBufferFBO); 	
	
	omniDepthShader.use();
	omniDepthShader.setUniform("far_plane", Light::FAR_PLANE);
	
	for (auto& light : pointLights) {
		if (light.dirtyShadows) {
			light.prepareShadowRendering(omniDepthShader);
			for (const auto& sceneObject : models) {
				sceneObject.second->depthRender(omniDepthShader);
			}
			light.dirtyShadows = false;
		}
	}
	
	depthShader.use();
	for (auto& light : spotLights) {
		if (light.dirtyShadows) {			
			light.prepareShadowRendering(depthShader);
			for (const auto& sceneObject : models) {
				sceneObject.second->depthRender(depthShader, light.getFrustum(), light.getLightSpaceMatrix());
			}
			light.dirtyShadows = false;
		}
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LightManager::updateParticles(float timeDelta) {
	for (auto& sl : spotLights) {
		sl.updateParticles(timeDelta);
	}
}

void LightManager::renderParticles(const glm::mat4& VP) {
	for (auto& sl : spotLights) {
		sl.renderParticles(VP);
	}
}

PointLight& LightManager::getPointLight(const unsigned int index)  {
	return pointLights[index];
}

SpotLight& LightManager::getSpotLight(const unsigned int index)  {
	return spotLights[index];
}

const std::vector<SpotLight>& LightManager::getSpotLights() const {
	return spotLights;
}

const std::vector<PointLight>& LightManager::getPointLights() const {
	return pointLights;
}


//called from shader
unsigned int LightManager::subscribeToLightUniforms() {
	lightUniformDirtySubscriptions.push_back(true);
	
	return lightUniformDirtySubscriptions.size() - 1;
}

//shader checking if uniforms are dirty
bool LightManager::isUniformDirty(unsigned int lightUniformSubscriptionID) const {
	return lightUniformDirtySubscriptions[lightUniformSubscriptionID];
}

//set all uniforms to dirty if any property changes
void LightManager::setAllUniformsDirty() {
	for (unsigned int i = 0; i < lightUniformDirtySubscriptions.size(); i++) {
		lightUniformDirtySubscriptions[i] = true;
	}
}

//called from shader after updating
void LightManager::onUniformUpdate(unsigned int lightUniformSubscriptionID) {
	lightUniformDirtySubscriptions[lightUniformSubscriptionID] = false;
}

void LightManager::updateLightTargets(float elapsedTime) {
	bool changesMade = false;
	for (auto& sl : spotLights) {
		changesMade = changesMade || sl.updateTarget(elapsedTime);
	}
	for (auto& pl : pointLights) {
		changesMade = changesMade || pl.updateTarget(elapsedTime);
	}
	if (changesMade) setAllUniformsDirty();
}

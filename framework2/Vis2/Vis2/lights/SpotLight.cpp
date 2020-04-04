#include <GL\glew.h> 
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\string_cast.hpp>
#include <string>
#include "Light.h"
#include <algorithm>

const glm::vec3 SpotLight::SPOT_LIGHT_DEFAULT_ENERGY{ glm::vec3{1.0f, 1.0f, 1.0f} };
const float SpotLight::SPOT_LIGHT_CONSTANT_VAL_DEFAULT = 1.0f;
const float SpotLight::SPOT_LIGHT_LINEAR_VAL_DEFAULT = 0.0005f;
const float SpotLight::SPOT_LIGHT_QUADRATIC_VAL_DEFAULT = 0.00005f;
const float SpotLight::SPOT_LIGHT_DEFAULT_CUTOFF = 8.0f;
const float SpotLight::SPOT_LIGHT_DEFAULT_OUTER_CUTOFF = 20.0f;
const float SpotLight::SPOT_LIGHT_DEFAULT_SHADOW_BIAS = 0.000013f;
const unsigned int SpotLight::SPOT_LIGHT_DEFAULT_SHADOW_SIZE = 1024;
unsigned int SpotLight::spotLightCount = 0;

SpotLight::SpotLight(const LightProperties& properties, LightManager& lightManager) : Light(properties, lightManager) {	
	spotLightIndex = spotLightCount;
	spotLightCount++;

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SPOT_LIGHT_DEFAULT_SHADOW_SIZE, SPOT_LIGHT_DEFAULT_SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);	
	glBindTexture(GL_TEXTURE_2D, 0);

	if (this->properties.constant == 0.0f) {
		this->properties.constant = SPOT_LIGHT_CONSTANT_VAL_DEFAULT;
	}
	if (this->properties.linear == 0.0f) {
		this->properties.linear = SPOT_LIGHT_LINEAR_VAL_DEFAULT;
	}
	if (this->properties.quadratic == 0.0f) {
		this->properties.quadratic = SPOT_LIGHT_QUADRATIC_VAL_DEFAULT;
	}
	if (this->properties.cutOff == 0.0f) {
		this->properties.cutOff = SPOT_LIGHT_DEFAULT_CUTOFF;
	}
	if (this->properties.outerCutOff == 0.0f) {
		this->properties.outerCutOff = SPOT_LIGHT_DEFAULT_OUTER_CUTOFF;
	}	
	if (this->properties.bias == 0.0f) {
		this->properties.bias = SPOT_LIGHT_DEFAULT_SHADOW_BIAS;
	}

	original = this->properties;

	updateLightSpaceMatrix();

	slps = std::make_shared<SLParticleSystem>();
	slps->init(this->properties.position, *(lightManager.particleUpdate.get()), 
		lightManager.particleRender, this->properties.direction, this->properties.cutOff, this->properties.outerCutOff, this->properties.energy.r);

	ftc.setCamInternals(properties.outerCutOff * 2, 1.0f, 0.01f, FAR_PLANE);
	ftc.setCamDef(properties.position, properties.position + properties.direction, glm::vec3(0.0, 1.0, 0.0));
}

SpotLight::SpotLight(const glm::vec3& position, const glm::vec3& direction, LightManager& lightManager)
	: SpotLight{ LightProperties{position, direction, SPOT_LIGHT_DEFAULT_ENERGY}, lightManager } {
}

void SpotLight::updateMatricesUniforms(const Shader& shader) const {
	std::string si = std::to_string(spotLightIndex);
	shader.use();
	shader.setUniform("lightProjectionMatrix[" + si + "]", lightProjection);	
	shader.setUniform("lightViewMatrix[" + si + "]", lightView);	
}

void SpotLight::modifyLight(const LightProperties& properties, const Shader& shader) {
	this->properties = properties;	

	if (this->properties.constant == 0.0f) {
		this->properties.constant = SPOT_LIGHT_CONSTANT_VAL_DEFAULT;
	}
	if (this->properties.linear == 0.0f) {
		this->properties.linear = SPOT_LIGHT_LINEAR_VAL_DEFAULT;
	}
	if (this->properties.quadratic == 0.0f) {
		this->properties.quadratic = SPOT_LIGHT_QUADRATIC_VAL_DEFAULT;
	}
	if (this->properties.cutOff == 0.0f) {
		this->properties.cutOff = SPOT_LIGHT_DEFAULT_CUTOFF;
	}
	if (this->properties.outerCutOff == 0.0f) {
		this->properties.outerCutOff = SPOT_LIGHT_DEFAULT_OUTER_CUTOFF;
	}

	updateLightSpaceMatrix();	
	dirtyShadows = true;
	if (properties.energy == glm::vec3(0.0f, 0.0f, 0.0f)) isActive = false;
	else isActive = true;
	lightManager.setAllUniformsDirty();
}

void SpotLight::modifyLight(const glm::vec3& position, const Shader& shader) {
	properties.position = position;
	updateLightSpaceMatrix();
	shader.use();
	shader.setUniform("spotLights[" + std::to_string(spotLightIndex) + "].position", properties.position);
	dirtyShadows = true;
	lightManager.setAllUniformsDirty();
}

void SpotLight::prepareShadowRendering(const Shader& shader) const {
	glViewport(0, 0, SPOT_LIGHT_DEFAULT_SHADOW_SIZE, SPOT_LIGHT_DEFAULT_SHADOW_SIZE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glClear(GL_DEPTH_BUFFER_BIT);
	shader.setUniform("lightSpaceMatrix", lightSpaceMatrix);
}

const glm::mat4& SpotLight::getLightSpaceMatrix() const {
	return lightSpaceMatrix;
}

void SpotLight::updateLightSpaceMatrix() {
	lightProjection = glm::perspective(glm::radians(properties.outerCutOff * 2), 1.0f, 0.01f, FAR_PLANE);
	lightView = glm::lookAt(properties.position, properties.position + properties.direction, glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
}

void SpotLight::updateParticles(float timeDelta) {
	slps->updatePositions(timeDelta);
}

void SpotLight::renderParticles(const glm::mat4& VP) {
	if (isActive && particleActive) slps->render(VP);
	slps->swapBuffers();
}

void SpotLight::setTarget(const LightProperties& targetProperties, float duration, float delay) {
	tweenData.push_back(LightTweenData{});
	LightTweenData& ltd = tweenData.back();
	ltd.targetProperties = targetProperties;
	ltd.initProperties = tweenData.size() > 1 ? tweenData[tweenData.size() - 2].targetProperties : properties;
	ltd.timeRemaining = duration;
	ltd.timeElapsed = 0.0f;
	ltd.delay = delay;	

	if (!targetProperties.positionChanging) {
		ltd.targetProperties.position = ltd.initProperties.position;
		ltd.targetProperties.direction = ltd.initProperties.direction;
	}
}

bool SpotLight::updateTarget(float deltaTime) {	
	if (tweenData.size() == 0) return false;

	LightTweenData& current = tweenData[0];

	if (current.delay > 0.0f) {
		current.delay -= deltaTime;
		if (current.delay < 0.0f) {
			deltaTime += current.delay;
		} else {
			return false;
		}
	}

	deltaTime = std::min(current.timeRemaining, deltaTime);
		
	if (current.timeRemaining - deltaTime <= 0.0f) {
		//properties.vp.tau = targetProperties.vp.tau;
		properties.vp.phi = current.targetProperties.vp.phi;

		properties.cutOff = current.targetProperties.cutOff;
		//properties.outerCutOff = current.targetProperties.outerCutOff;

		properties.energy = current.targetProperties.energy;

		if (current.targetProperties.positionChanging) {
			properties.direction = current.targetProperties.direction;
			properties.position = current.targetProperties.position;
			updateLightSpaceMatrix();
			dirtyShadows = true;
		}

		current.timeRemaining = 0.0f;

		tweenData.erase(tweenData.begin());

	} else {

		float addFactor = deltaTime / (current.timeElapsed + current.timeRemaining);
		//properties.vp.tau += (targetProperties.vp.tau - initProperties.vp.tau) * addFactor;
		properties.vp.phi += (current.targetProperties.vp.phi - current.initProperties.vp.phi) * addFactor;

		properties.cutOff += (current.targetProperties.cutOff - current.initProperties.cutOff) * addFactor;
		//properties.outerCutOff += (current.targetProperties.outerCutOff - current.initProperties.outerCutOff) * addFactor;

		properties.energy += (current.targetProperties.energy - current.initProperties.energy) * addFactor;

		if (current.targetProperties.positionChanging) {
			properties.direction += (current.targetProperties.direction - current.initProperties.direction) * addFactor;
			properties.position += (current.targetProperties.position - current.initProperties.position) * addFactor;
			dirtyShadows = true;
			updateLightSpaceMatrix();
		}

		current.timeRemaining -= deltaTime;
	}

	current.timeElapsed += deltaTime;	

	if (properties.energy == glm::vec3(0.0f, 0.0f, 0.0f)) isActive = false;
	else isActive = true;

	slps->setLightProperties(properties.cutOff, properties.outerCutOff, properties.energy.x, properties.position, properties.direction);

	return true;
}

void SpotLight::turnOn(float duration) {	
	setTarget(original, duration, 0.0f);
}

void SpotLight::turnOnfromInactive(float duration) {
	LightProperties lp;
	lp.vp.phi = 0;
	lp.cutOff =0;
	lp.outerCutOff = 0;
	lp.energy = properties.energy * 0.0f;
	setTarget(lp, 0.0f, 0.0f);
	setTarget(original, duration, 0.0f);
}

void SpotLight::turnOff(float duration) {
	LightProperties lp;
	lp.vp.phi = 0;
	lp.cutOff = 0;
	lp.outerCutOff = 0;
	lp.energy = properties.energy * 0.0f;
	setTarget(lp, duration, 0.0f);
}

void SpotLight::flickerOnFromInactive() {
	LightProperties lp;
	lp.vp.phi = properties.vp.phi/5;
	lp.cutOff = properties.cutOff / 4;
	lp.outerCutOff = properties.outerCutOff / 2;
	lp.energy = properties.energy * 0.8f;
	setTarget(lp, 0.05f, 0.0f);
	lp.cutOff = properties.cutOff / 4 + 1;
	lp.outerCutOff = properties.outerCutOff - 5;
	lp.energy = properties.energy * 0.5f;
	lp.vp.phi = properties.vp.phi / 10;
	setTarget(lp, 0.05f);
	lp.vp.phi = properties.vp.phi / 2.5;
	lp.cutOff = properties.cutOff / 4 + 2;
	lp.outerCutOff = properties.outerCutOff - 4;
	lp.energy = properties.energy;
	setTarget(lp, 0.05f);
	lp.cutOff = properties.cutOff / 4 + 3;
	lp.outerCutOff = properties.outerCutOff - 4;
	lp.energy = properties.energy * 0.7f;
	lp.vp.phi = properties.vp.phi / 10.0f;
	setTarget(lp, 0.5f);
	lp.vp.phi = properties.vp.phi;
	lp.cutOff = properties.cutOff;
	lp.outerCutOff = properties.outerCutOff;;
	lp.energy = properties.energy;

	setTarget(lp, 0.05f, 0.15f);
}

void SpotLight::flickerOnOff() {
	LightProperties lp;
	lp.vp.phi = properties.vp.phi / 5;
	lp.cutOff = properties.cutOff / 4;
	lp.outerCutOff = properties.outerCutOff / 2;
	lp.energy = properties.energy * 0.8f;
	setTarget(lp, 0.05f);
	lp.cutOff = properties.cutOff / 4 + 1;
	lp.outerCutOff = properties.outerCutOff - 5;
	lp.energy = properties.energy * 0.5f;
	lp.vp.phi = properties.vp.phi / 10;
	setTarget(lp, 0.05f);
	lp.vp.phi = properties.vp.phi / 2.5;
	lp.cutOff = properties.cutOff / 4 + 2;
	lp.outerCutOff = properties.outerCutOff - 4;
	lp.energy = properties.energy;
	setTarget(lp, 0.05f);
	lp.cutOff = 0;
	lp.outerCutOff = 0;
	lp.energy = properties.energy * 0.0f;
	lp.vp.phi = 0;
	setTarget(lp, 0.1f);	
}

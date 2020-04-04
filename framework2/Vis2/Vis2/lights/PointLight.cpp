#include <GL\glew.h> 
#include <glm\gtc\matrix_transform.hpp>
#include <string>
#include <algorithm>
#include "Light.h"


const glm::mat4 PointLight::SHADOW_PROJ = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, FAR_PLANE);
const glm::vec3 PointLight::POINT_LIGHT_DEFAULT_ENERGY{ glm::vec3{1.0f, 1.0f, 1.0f} };
const float PointLight::POINT_LIGHT_CONSTANT_VAL_DEFAULT = 1.0f;
const float PointLight::POINT_LIGHT_LINEAR_VAL_DEFAULT = 0.01f;
const float PointLight::POINT_LIGHT_QUADRATIC_VAL_DEFAULT = 0.0031f;
const float PointLight::POINT_LIGHT_DEFAULT_SHADOW_BIAS = 0.2f;
const unsigned int PointLight::POINT_LIGHT_DEFAULT_SHADOW_SIZE = 2048;
unsigned int PointLight::pointLightCount = 0;

PointLight::PointLight(const LightProperties& properties, LightManager& lightManager) : Light(properties, lightManager) {		
	pointLightIndex = pointLightCount;
	pointLightCount++;

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthMap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, POINT_LIGHT_DEFAULT_SHADOW_SIZE, POINT_LIGHT_DEFAULT_SHADOW_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);	
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (this->properties.constant == 0.0f) {
		this->properties.constant = POINT_LIGHT_CONSTANT_VAL_DEFAULT;
	}
	if (this->properties.linear == 0.0f) {
		this->properties.linear = POINT_LIGHT_LINEAR_VAL_DEFAULT;
	}
	if (this->properties.quadratic == 0.0f) {
		this->properties.quadratic = POINT_LIGHT_QUADRATIC_VAL_DEFAULT;
	}
	if (this->properties.bias == 0.0f) {
		this->properties.bias = POINT_LIGHT_DEFAULT_SHADOW_BIAS;
	}

	original = this->properties;

	computeCubeShadowTransform();
}
PointLight::PointLight(const glm::vec3& position, LightManager& lightManager)
	: PointLight(LightProperties{ position, POINT_LIGHT_DEFAULT_ENERGY }, lightManager) {
}

void PointLight::modifyLight(const LightProperties& props, const Shader& shader) {

	properties = props;

	if (this->properties.constant == 0.0f) {
		this->properties.constant = POINT_LIGHT_CONSTANT_VAL_DEFAULT;
	}
	if (this->properties.linear == 0.0f) {
		this->properties.linear = POINT_LIGHT_LINEAR_VAL_DEFAULT;
	}
	if (this->properties.quadratic == 0.0f) {
		this->properties.quadratic = POINT_LIGHT_QUADRATIC_VAL_DEFAULT;
	}
	computeCubeShadowTransform();	
	dirtyShadows = true;
	lightManager.setAllUniformsDirty();
	
}

void PointLight::modifyLight(const glm::vec3& position, const Shader& shader) {
	properties.position = position;

	shader.use();
	shader.setUniform("pointLights[" + std::to_string(pointLightIndex) + "].position", properties.position);
	computeCubeShadowTransform();
	dirtyShadows = true;
	lightManager.setAllUniformsDirty();
}

void PointLight::prepareShadowRendering(const Shader& shader) const {
	glViewport(0, 0, POINT_LIGHT_DEFAULT_SHADOW_SIZE, POINT_LIGHT_DEFAULT_SHADOW_SIZE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
	glClear(GL_DEPTH_BUFFER_BIT);
	shader.setUniform("lightPos", properties.position);
	for (unsigned int j = 0; j < 6; j++) {
		shader.setUniform("shadowMatrices[" + std::to_string(j) + "]", cubeShadowTransform[j]);
	}
}

void PointLight::computeCubeShadowTransform() {	
	cubeShadowTransform.clear();
	cubeShadowTransform.push_back(SHADOW_PROJ * glm::lookAt(properties.position, properties.position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	cubeShadowTransform.push_back(SHADOW_PROJ * glm::lookAt(properties.position, properties.position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	cubeShadowTransform.push_back(SHADOW_PROJ * glm::lookAt(properties.position, properties.position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	cubeShadowTransform.push_back(SHADOW_PROJ * glm::lookAt(properties.position, properties.position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	cubeShadowTransform.push_back(SHADOW_PROJ * glm::lookAt(properties.position, properties.position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	cubeShadowTransform.push_back(SHADOW_PROJ * glm::lookAt(properties.position, properties.position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));	
}

void PointLight::setTarget(const LightProperties& targetProperties, float duration, float delay) {
	tweenData.push_back(LightTweenData{});
	LightTweenData& ltd = tweenData.back();
	ltd.targetProperties = targetProperties;
	ltd.initProperties = tweenData.size() > 1 ? tweenData[tweenData.size() - 2].targetProperties : properties;
	ltd.timeRemaining = duration;
	ltd.timeElapsed = 0.0f;
	ltd.delay = delay;
}
bool PointLight::updateTarget(float deltaTime) {
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

		properties.energy = current.targetProperties.energy;
		current.timeRemaining = 0.0f;
		tweenData.erase(tweenData.begin());

	} else {

		float addFactor = deltaTime / (current.timeElapsed + current.timeRemaining);	
		properties.energy += (current.targetProperties.energy - current.initProperties.energy) * addFactor;
		current.timeRemaining -= deltaTime;
	}

	current.timeElapsed += deltaTime;
	
	if (properties.energy == glm::vec3(0.0f, 0.0f, 0.0f)) isActive = false;
	else isActive = true;

	return true;
}

void PointLight::turnOnfromInactive(float duration) {
	LightProperties lp;
	lp.energy = properties.energy * 0.0f;
	setTarget(lp, 0.0f, 0.0f);
	lp.energy = properties.energy;
	setTarget(lp, duration, 0.0f);
}

void PointLight::turnOn(float duration) {
	setTarget(original, duration, 0.0f);
}

void PointLight::turnOff(float duration) {
	LightProperties lp;
	lp.vp.phi = 0;
	lp.cutOff = 0;
	lp.outerCutOff = 0;
	lp.energy = properties.energy * 0.0f;
	setTarget(lp, duration, 0.0f);
}
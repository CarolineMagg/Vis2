#pragma once
#include "..\shading\Shader.h"
#include <glm\glm.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include "..\SceneObject.h"
#include "SLParticleSystem.h"

struct VolumeProperties {
	bool hasVolume{ false };
	float tau{ 0.01f };
	int phi{ 100000 };
	VolumeProperties() {}
	VolumeProperties(bool hasVolume, float tau, unsigned int phi) :
		hasVolume(hasVolume), tau(tau), phi(phi) {
	}
};

struct LightProperties {
	glm::mat4 lastLightSpaceMatrix;
	glm::vec3 direction;
	glm::vec3 position;	
	glm::vec3 energy;	
	float cutOff{ 0.0f };
	float outerCutOff{ 0.0f };
	float constant{ 0.0f };
	float linear{ 0.0f };
	float quadratic{ 0.0f };
	float bias{ 0.0 };	
	VolumeProperties vp;
	bool positionChanging = false;

	LightProperties() {}
	LightProperties(const glm::vec3& position, const glm::vec3& energy) :
		position(position), energy(energy) {
	}
	LightProperties(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& energy) :
		position(position), direction(direction), energy(energy) {
	}
	LightProperties(const glm::vec3& position, const glm::vec3& energy, VolumeProperties& vp) :
		position(position), energy(energy), vp(vp) {
	}
	LightProperties(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& energy, const VolumeProperties& vp) :
		position(position), direction(direction), energy(energy), vp(vp) {
	}
};

struct LightTweenData {
	LightProperties targetProperties;
	LightProperties initProperties;
	float timeElapsed;
	float timeRemaining;
	float delay;
};

class LightManager;

class Light {
public:	
	
	const static float FAR_PLANE;
	
	virtual ~Light() = default;

	void virtual modifyLight(const LightProperties& properties, const Shader& shader) = 0;
	void virtual modifyLight(const glm::vec3& position, const Shader& shader) = 0;	
	void virtual prepareShadowRendering(const Shader& shader) const = 0;	

	const LightProperties& getProperties() const { return properties; }
			
	unsigned int depthMap;
	bool dirtyShadows = true;

	bool isActive = true;
	LightProperties original;

	void virtual setTarget(const LightProperties& properties, float duration, float delay = 0.0f) = 0;
	bool virtual updateTarget(float deltaTime) = 0;
protected:		
	Light(const LightProperties& properties, LightManager& lightManager);
	LightProperties properties;	
	
	LightManager& lightManager;
	

	std::vector<LightTweenData> tweenData;
};

class SpotLight : public Light {
public:

	static const glm::vec3 SPOT_LIGHT_DEFAULT_ENERGY;	

	static const float SPOT_LIGHT_CONSTANT_VAL_DEFAULT;
	static const float SPOT_LIGHT_LINEAR_VAL_DEFAULT;
	static const float SPOT_LIGHT_QUADRATIC_VAL_DEFAULT;

	static const float SPOT_LIGHT_DEFAULT_CUTOFF;
	static const float SPOT_LIGHT_DEFAULT_OUTER_CUTOFF;

	static const float SPOT_LIGHT_DEFAULT_SHADOW_BIAS;
	static const unsigned int SPOT_LIGHT_DEFAULT_SHADOW_SIZE;

	SpotLight(const LightProperties& properties, LightManager& lightManager);
	SpotLight(const glm::vec3& position, const glm::vec3& direction, LightManager& lightManager);

	void virtual modifyLight(const LightProperties& properties, const Shader& shader) override;
	void virtual modifyLight(const glm::vec3& position, const Shader& shader) override;	
	void virtual prepareShadowRendering(const Shader& shader) const override;	
	const glm::mat4& getLightSpaceMatrix() const;

	void updateMatricesUniforms(const Shader& shader) const;

	void updateParticles(float timeDelta);
	void renderParticles(const glm::mat4& VP);
	bool particleActive = true;

	void turnOn(float duration);
	void turnOnfromInactive(float duration);
	void turnOff(float duration);
	void flickerOnFromInactive();
	void flickerOnOff();

	const FrustumCull& getFrustum() { return ftc; }

	void virtual setTarget(const LightProperties& properties, float duration, float delay = 0.0f) override;
	bool virtual updateTarget(float deltaTime) override;
	
private:
	static unsigned int spotLightCount;
	unsigned int spotLightIndex = 0;
	glm::mat4 lightSpaceMatrix;
	glm::mat4 lightView;
	glm::mat4 lightProjection;
	std::shared_ptr<SLParticleSystem> slps;
	
	FrustumCull ftc;

	void updateLightSpaceMatrix();	
};

class PointLight : public Light {

public:
	static const glm::mat4 SHADOW_PROJ;
	static const glm::vec3 POINT_LIGHT_DEFAULT_ENERGY;	

	static const float POINT_LIGHT_CONSTANT_VAL_DEFAULT;
	static const float POINT_LIGHT_LINEAR_VAL_DEFAULT;
	static const float POINT_LIGHT_QUADRATIC_VAL_DEFAULT;

	static const float POINT_LIGHT_DEFAULT_SHADOW_BIAS;
	static const unsigned int POINT_LIGHT_DEFAULT_SHADOW_SIZE;
	
	PointLight(const LightProperties& properties, LightManager& lightManager);
	PointLight(const glm::vec3& position, LightManager& lightManager);
	void virtual modifyLight(const LightProperties& props, const Shader& shader) override;
	void virtual modifyLight(const glm::vec3& position, const Shader& shader) override;	
	void virtual prepareShadowRendering(const Shader& shader) const override;

	void virtual setTarget(const LightProperties& properties, float duration, float delay = 0.0f) override;
	bool virtual updateTarget(float deltaTime) override;

	void turnOn(float duration);
	void turnOnfromInactive(float duration);
	void turnOff(float duration);

private:
	static unsigned int pointLightCount;
	unsigned int pointLightIndex = 0;
	
	std::vector<glm::mat4> cubeShadowTransform;

	void computeCubeShadowTransform();
};

class LightManager {
public:
	LightManager();
	int addSpotLight(const LightProperties& properties);
	int addSpotLight(glm::vec3 position, glm::vec3 energy, glm::vec3 direction, float cutOff, float outerCutoff);
	int addPointLight(const LightProperties& properties);
	int addPointLight(glm::vec3 position, glm::vec3 energy);		
	void renderDepthMaps(const std::unordered_map<std::string, std::shared_ptr<SceneObject>>& models);
	void updateParticles(float timeDelta);
	void renderParticles(const glm::mat4& VP);

	void updateLightTargets(float elapsedTime);

	SpotLight& getSpotLight(const unsigned int index);
	PointLight& getPointLight(const unsigned int index);
	const std::vector<SpotLight>& getSpotLights() const;
	const std::vector<PointLight>& getPointLights() const;

	unsigned int subscribeToLightUniforms();	
	bool isUniformDirty(unsigned int lightUniformSubscriptionID) const;	
	void setAllUniformsDirty();	
	void onUniformUpdate(unsigned int lightUniformSubscriptionID);

	unsigned int depthTextureCount = 0;

	friend class SpotLight;
	friend class pointLight;
private:
	std::vector<SpotLight> spotLights;
	std::vector<PointLight> pointLights;	
	Shader depthShader;
	Shader omniDepthShader;
	Shader particleRender;
	std::shared_ptr<Shader> particleUpdate;

		

	std::vector<bool> lightUniformDirtySubscriptions;	

	unsigned int depthBufferFBO;
};
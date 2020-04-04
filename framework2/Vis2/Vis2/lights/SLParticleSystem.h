#pragma once
#include <GL\glew.h> 
#include <glm\glm.hpp>

#include "../shading/Shader.h"

class SLParticleSystem {
public:
	SLParticleSystem();
	~SLParticleSystem();
	bool init(const glm::vec3& Pos, Shader &updateShader, Shader &drawShader, const glm::vec3& direction, float cutOff, float outerCutOff, float lightPower);
	
	void updatePositions(float timeDelta);
	void render(const glm::mat4& VP);
	void swapBuffers();

	void setLightProperties(float _cutOff, float _outerCutOff, float _lPower, const glm::vec3& pos, const glm::vec3& direction) {
		cutOff = glm::cos(glm::radians(_cutOff));
		outerCutOff = glm::cos(glm::radians(_outerCutOff));
		lightPower = _lPower;
		lightDirection = direction;
		position = pos;
	};

private:
	unsigned int VAO1;

	Shader *updateShader;
	Shader *drawShader;	

	unsigned int initialDrawCounts = 0;

	unsigned int currentB;
	unsigned int currentTFBuffer;
	float totalTime = 0.0f;
	GLuint partBuffer[2];
	GLuint tFeedback[2];
		
	glm::vec3 lightDirection;
	glm::vec3 position;
	float cutOff;
	float outerCutOff;
	float emissionOuterCutOff;
	float lightPower = 1.0f;

};

struct Particle {
	float Type;
	glm::vec3 Pos;
	glm::vec3 Vel;
	float LifetimeMillis;	
}; 

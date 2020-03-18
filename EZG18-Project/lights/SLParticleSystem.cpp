
#include "SLParticleSystem.h"
#include <iostream>
#include <cstdlib>

#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>


SLParticleSystem::SLParticleSystem() {
	currentB = 0;
	currentTFBuffer = 1;	
	totalTime = 0;

	memset(tFeedback, 0, sizeof(tFeedback));
	memset(partBuffer, 0, sizeof(partBuffer));
}


SLParticleSystem::~SLParticleSystem() {	
	if (tFeedback[0] != 0) {
		glDeleteTransformFeedbacks(2, tFeedback);
	}

	if (partBuffer[0] != 0) {
		glDeleteBuffers(2, partBuffer);
	}
}


bool SLParticleSystem::init(const glm::vec3& position, Shader &updateShader, Shader &drawShader, const glm::vec3& direction, float cutOff, float outerCutOff, float lightPower) {
	Particle Particles[1000];
	memset(Particles, 0, sizeof(Particles));

	Particles[0].Type = 0.0f;
	Particles[0].Pos = position;
	Particles[0].Vel = glm::vec3(0.0f, 0.0f, 0.0f);
	Particles[0].LifetimeMillis = 1.0f;

	glGenVertexArrays(1, &VAO1);
	glBindVertexArray(VAO1);

	glGenTransformFeedbacks(2, tFeedback);
	glGenBuffers(2, partBuffer);

	for (unsigned int i = 0; i < 2; i++) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tFeedback[i]);
		glBindBuffer(GL_ARRAY_BUFFER, partBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), Particles, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, partBuffer[i]);
	}
	
	this->lightDirection = glm::normalize(direction);
	this->cutOff = glm::cos(glm::radians(cutOff));
	this->outerCutOff = glm::cos(glm::radians(outerCutOff));
	this->emissionOuterCutOff = this->outerCutOff;
	this->position = position;
	this->lightPower = lightPower;

	updateShader.use();
	updateShader.setUniform("launcherDur", 0.01f);
	updateShader.setUniform("shellDur", 10000.0f);	
		
	this->updateShader = &updateShader;
	this->drawShader = &drawShader;
	
	

	glBindVertexArray(0);

	return true;
}


void SLParticleSystem::updatePositions(float timeDelta) {
	timeDelta *= 1000;
	
	if (initialDrawCounts < 15) {
		timeDelta *= 100;
		totalTime += timeDelta;
		if (initialDrawCounts == 0) {
			srand((unsigned int)timeDelta*totalTime);
		}
		initialDrawCounts++;
		if (initialDrawCounts == 15) {
			updateShader->use();
			updateShader->setUniform("launcherDur", 90.0f);
		}
	} else {
		totalTime += timeDelta;
	}

	glm::vec4 randoms(
		static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
		static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
		static_cast <float> (rand()) / static_cast <float> (RAND_MAX),
		((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2 - 1) * 0.2f
	);

	glm::vec3 base{ 0.0f, 0.0f, 1.0f };	
	glm::vec3 u = glm::normalize(glm::cross(base, lightDirection));
	float rot = acos(glm::dot(lightDirection, base));
	glm::mat3 R = glm::rotate(glm::mat4{ 1.0f }, rot, u);

	updateShader->use();
	updateShader->setUniform("timeDelta", timeDelta);
	updateShader->setUniform("randoms", randoms);
	updateShader->setUniform("dirRot", R);	
	updateShader->setUniform("lightDir", lightDirection);
	updateShader->setUniform("lightPos", position);
	updateShader->setUniform("cutOff", this->emissionOuterCutOff);


	glEnable(GL_RASTERIZER_DISCARD);

	glBindVertexArray(VAO1);
	
	glBindBuffer(GL_ARRAY_BUFFER, partBuffer[currentB]);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tFeedback[currentTFBuffer]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);                          // type
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);         // position
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);        // velocity
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)28);          // lifetime

	glBeginTransformFeedback(GL_POINTS);

	if (initialDrawCounts == 1) {
		glDrawArrays(GL_POINTS, 0, 1);
	} else {
		glDrawTransformFeedback(GL_POINTS, tFeedback[currentB]);
	}

	glEndTransformFeedback();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	glBindVertexArray(0);

	glDisable(GL_RASTERIZER_DISCARD);
}


void SLParticleSystem::render(const glm::mat4& VP) {
	drawShader->use();
	drawShader->setUniform("VP", VP);	
	drawShader->setUniform("lightDir", lightDirection);
	drawShader->setUniform("lightPos", position);
	drawShader->setUniform("cutOff", this->outerCutOff);
	drawShader->setUniform("iC", this->cutOff);
	drawShader->setUniform("lightPower", this->lightPower);

	glBindVertexArray(VAO1);
	glBindBuffer(GL_ARRAY_BUFFER, partBuffer[currentTFBuffer]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);  // position
	glDrawTransformFeedback(GL_POINTS, tFeedback[currentTFBuffer]);
	glDisableVertexAttribArray(0);
	glBindVertexArray(0);	
}

void SLParticleSystem::swapBuffers() {
	currentB = currentTFBuffer;
	currentTFBuffer = (currentTFBuffer + 1) & 0x1;
}

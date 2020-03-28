#include <gl3w.h>
#include "Scene.h"
#include <iostream>
#include <algorithm>
#include "mesh/Mesh.h"


extern void renderQuad();

unsigned int Scene::WIDTH_DEFAULT = 1280;
unsigned int Scene::HEIGHT_DEFAULT = 720;


Scene::Scene(const Camera& camera) : camera(camera)
{
	lightManager = std::make_shared<LightManager>();
	mainShader = std::make_shared<MainShader>(*lightManager);
	volShader = std::make_unique<VolumetricLightShader>(*lightManager);

	// keeping these for job examples
	/*
	addToJobChain(std::move(Job<void>{1.0f, [this]() {
		LightProperties lp = lightManager->getSpotLight(0).getProperties();
		lp.position = glm::vec3{ -12.9301,-1.12391,18.1471 };
		lp.direction = glm::vec3(0.547853, 0.0401318, -0.835612);
		lp.positionChanging = true;
		lightManager->getSpotLight(0).setTarget(lp, 0.0f, 0.0f);

		lightManager->getSpotLight(0).turnOn(3.5f);
		lp = lightManager->getSpotLight(0).original;
		lp.position = glm::vec3{ -10.6695,-0.673713,14.9962 };
		lp.direction = glm::vec3(0.440152, 0.0784588, -0.894489);
		lp.positionChanging = true;
		lightManager->getSpotLight(0).setTarget(lp, 5, 0.0f);
		
		lp.position = glm::vec3{ -3.7,0.61,15.6 };
		lp.direction = glm::vec3(-0.15, -0.132747, -0.97);
		lp.positionChanging = true;
		lightManager->getSpotLight(0).setTarget(lp, 5, 0.0f);
	}}));
	addToJobChain(std::move(Job<void>{13.0f, [this]() {lightManager->getSpotLight(0).turnOff(1.5f);}}));

	*/

	/*addToJobChain(std::move(Job<void>{5.0f, [&bmt = blendMainTexture, &twf = tweeningFloats]() {
		TweenVariableData<float> tt{ &bmt, 1.0f, 5.0f };
		twf.push_back(tt);
	}}));*/

	/*addToJobChain(std::move(Job<void>{7.0f, [&bmt = blendMainTexture, &twf = tweeningFloats]() {
		TweenVariableData<float> tt{ &bmt, 0.0f, 10.0f };
		twf.push_back(tt);
	}}));*/


}

void Scene::add(std::string key, std::shared_ptr<SceneObject>&& obj)
{
	sceneObject.emplace(key, std::move(obj));
}


SceneObject* Scene::get(std::string id)
{
	return sceneObject.at(id).get();
}

void Scene::render()
{	

	if (blendMainTexture == 0.0f) {
		Mesh::renderMode = RenderMode::NORMAL;
	}
	else if (blendMainTexture < 1.0) {
		Mesh::renderMode = RenderMode::NORMAL_TAM;
	}
	else {
		Mesh::renderMode = RenderMode::TAM;
	}

	mainRenderFBO.setActive();

	for (auto& elements : sceneObject)
	{
		elements.second->render(lightManager->depthTextureCount, *mainShader, camera.ftc);
	}

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	lightManager->renderParticles(camera.getProjection() * camera.getViewMatrix());

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	unsigned int numSpot = 0;
	for (auto& sp : lightManager->getSpotLights()) {
		if (sp.isActive) numSpot++;
	}

	if (numSpot > 0) {
		volShader->setupRender(*lightManager, camera, mainRenderFBO.getDepthTexture());
		volLightOutputFBO.setActive();
		renderQuad();

		//blur

		blurShader.use();
		glActiveTexture(GL_TEXTURE0);
		blurShader.setUniform("u_texture", 0);
		glBindTexture(GL_TEXTURE_2D, volLightOutputFBO.getColorTexture());
		blurShader.setUniform("dir", glm::vec2(0.0f, 1.0f));
		blurVerticalOutputFBO.setActive();
		renderQuad();

		glActiveTexture(GL_TEXTURE0);
		blurShader.setUniform("u_texture", 0);
		glBindTexture(GL_TEXTURE_2D, blurVerticalOutputFBO.getColorTexture());
		blurShader.setUniform("dir", glm::vec2(1.0f, 0.0f));
		blurHorizontalOutputFBO.setActive();
		renderQuad();
	}
	

	//Contours
	if (contourBlend > 0.0f) {
		contourShader.use();
		glActiveTexture(GL_TEXTURE0);
		contourShader.setUniform("depthInformation", 0);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mainRenderFBO.getDepthTexture());
		glActiveTexture(GL_TEXTURE0 + 1);
		contourShader.setUniform("normalInformation", 1);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mainRenderFBO.getColorTexture(2));
		contourShader.setUniform("nearZ", camera.nearPlane);
		contourShader.setUniform("farZ", camera.farPlane);
		contouringMask.setActive();
		renderQuad();
	}

	//Combine all the textures
	combineShader.use();
	glActiveTexture(GL_TEXTURE0);
	combineShader.setUniform("normalTexRender", 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mainRenderFBO.getColorTexture(0));
	glActiveTexture(GL_TEXTURE0 + 1);
	combineShader.setUniform("tamRender", 1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mainRenderFBO.getColorTexture(1));
	glActiveTexture(GL_TEXTURE0 + 2);
	combineShader.setUniform("volumetricLights", 2);
	glBindTexture(GL_TEXTURE_2D, blurHorizontalOutputFBO.getColorTexture());
	//Contour mask
	glActiveTexture(GL_TEXTURE0 + 3);
	combineShader.setUniform("contourMask", 3);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, contouringMask.getColorTexture());
	//WHere should we set and update these
	combineShader.setUniform("blendMainTexture", blendMainTexture);
	combineShader.setUniform("contourBlend", contourBlend);
	combineShader.setUniform("MSCount", FBO::MULTI_SAMPLE_COUNT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	renderQuad();

	//glActiveTexture(GL_TEXTURE0);
	glEnable(GL_BLEND);	
	glEnable(GL_DEPTH_TEST);
}

void Scene::preRender(float timeDelta)
{	
	lightManager->renderDepthMaps(sceneObject);
	if (timeDelta + pooledTweenTime < tweenUpdatesPerFrame) {
		pooledTweenTime += timeDelta;		
	} else {
		pooledTweenTime += timeDelta;
		updateJobs(pooledTweenTime);
		updateVariableTweens(pooledTweenTime);
		lightManager->updateLightTargets(pooledTweenTime);		
		pooledTweenTime = 0;
	}	
	lightManager->updateParticles(timeDelta);
		
	mainShader->setupRender(*lightManager, camera);
}

void Scene::updateJobs(float deltaTime) {
	if (jobsChain_void.size() == 0) return;	
	Job<void>& j = jobsChain_void[0];
	j.delay -= deltaTime;
	if (j.delay <= 0.0f) {
		j.func();
		deltaTime += j.delay;
		jobsChain_void.erase(jobsChain_void.begin());
		if (deltaTime > 0.0f) {
			updateJobs(deltaTime);
		}
	}
}

void Scene::addToJobChain(Job<void> &&job) {
	jobsChain_void.push_back(std::move(job));
}

void Scene::updateVariableTweens(float deltaTime) {
	for (unsigned int i = 0; i < tweeningFloats.size(); i++) {
		auto& d = tweeningFloats[i];
		*(d.variable) += (std::min(deltaTime, d.remainingTime) / d.remainingTime) * (d.end - *(d.variable));

		d.remainingTime -= deltaTime;

		if (d.remainingTime <= 0.0) {
			*(d.variable) = d.end;
			tweeningFloats.erase(tweeningFloats.begin() + i);
			i--;
		}
	}
}

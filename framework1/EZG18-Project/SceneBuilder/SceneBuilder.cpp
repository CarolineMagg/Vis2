#include "..\mesh\Model.h"
#include <glm\gtc\matrix_transform.hpp>
#include "SceneBuilder.h"
#include "..\shading\Shader.h"
#include <unordered_map>
#include "../SceneObjectGeometry.h"


std::unique_ptr<Scene> SceneBuilder::buildScene(const Camera& camera) {
	std::unique_ptr<Scene> theScene = std::make_unique<Scene>(camera);
	std::unordered_map<std::string, std::shared_ptr<Model>> models;

	//setup scene lights	
	theScene->getLightManager().addPointLight(LightProperties{ glm::vec3(-10.9087,4.79055,-1.31333), glm::vec3(1.0f, .9f, .9f) });
	theScene->getLightManager().addPointLight(glm::vec3(10.4277, 4.78483, -0.128228), glm::vec3(1.0f, .9f, .9f));	
	
	theScene->getLightManager().addSpotLight(LightProperties{ glm::vec3{6.95, 17.0, 10}, glm::vec3(-0.54f, -0.83f, -0.09f), glm::vec3(1.0f,1.0f, 1.0f),  VolumeProperties{true, 0.01f, 20000} });
	//theScene->getLightManager().getSpotLight(0).isActive = false;

	//models
	std::shared_ptr<Model> dojoModel = std::make_shared<Model>("models/scene/ezg_scene_start.dae");
	models.insert(std::make_pair("dojo_model", std::move(dojoModel)));

	glm::mat4 modelMatrixRoom = glm::rotate(glm::mat4(1.0f), -glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	theScene->add("first_room", std::make_shared<SceneObjectGeometry>(models.at("dojo_model"),modelMatrixRoom));

	return std::move(theScene);
}

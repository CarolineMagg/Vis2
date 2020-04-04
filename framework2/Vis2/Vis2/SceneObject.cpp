#include <string>
#include "SceneObject.h"

unsigned long SceneObject::counter = 0;

SceneObject::SceneObject()
{
	this->id = std::to_string(++counter)+"_automatic_id";
}

SceneObject::SceneObject(std::string id)
{
	++counter;
	this->id = id;
}

SceneObject::~SceneObject()
{
	children.clear();
}

std::string SceneObject::getId()
{
	return id;
}

void SceneObject::addChild(std::unique_ptr<SceneObject> obj)
{
	children.insert({ obj->getId(), std::move(obj) });
}

void SceneObject::removeChild(std::string id)
{
	children.erase(id);
}

void SceneObject::render(int texturesAdded, const Shader& shader, const FrustumCull& ftc, const glm::mat4& parentMatrix)
{
	for (auto& element : children) {
		element.second->render(texturesAdded, shader, ftc, parentMatrix);
	}
}

void SceneObject::depthRender(const Shader & shader)
{
	for (auto& element : children) {
		element.second->depthRender(shader);
	}
}

void SceneObject::depthRender(const Shader & shader, const FrustumCull& ftc, const glm::mat4& lightMatrix) {
	for (auto& element : children) {
		element.second->depthRender(shader, ftc, lightMatrix);
	}
}

void SceneObject::update(double deltaT)
{
	for (auto& element : children) {
		element.second->update(deltaT);
	}
}

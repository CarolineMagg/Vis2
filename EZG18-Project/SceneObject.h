#pragma once
#include <memory>
#include <unordered_map>
#include <glm\glm.hpp>
#include "./shading/Shader.h"
#include "./render/FrustumCull.h"


//TODO: implement method
//TODO: maybe some super class so we can also have transformation/animation nodes
//TODO: id service
class SceneObject
{
private:
	static unsigned long counter;
	std::string id;
	std::unordered_map<std::string, std::unique_ptr<SceneObject>> children;
public:
	SceneObject();
	SceneObject(std::string id);
	~SceneObject();
	std::string getId();
	void addChild(std::unique_ptr<SceneObject> obj);
	void removeChild(std::string id);

	virtual void render(int texturesAdded, const Shader& shader, const FrustumCull& ftc, const glm::mat4& parentMatrix = glm::mat4(1.0f));
	virtual void depthRender(const Shader &shader);
	virtual void depthRender(const Shader &shader, const FrustumCull& ftc, const glm::mat4& lightMatrix);
	virtual void update(double deltaT);
};


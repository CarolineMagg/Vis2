#pragma once
#include "SceneObject.h"
#include ".\mesh\Model.h"
#include "./shading/Shader.h"

class SceneObjectGeometry :
	public SceneObject
{
protected:
	glm::mat4 modelMatrix;
private:
	std::shared_ptr<Model> model;	
	glm::mat4 transformationMatrix = glm::mat4(1.0f);
	float alpha;
public:
	SceneObjectGeometry(std::shared_ptr<Model> model, const glm::mat4& modelMatrix, float alpha);
	SceneObjectGeometry(std::shared_ptr<Model> model, const glm::mat4& modelMatrix);
	~SceneObjectGeometry();

	void transform(glm::mat4 transformation);
	void resetTransformationMatrix();
	void setModelMatrix(glm::mat4 modelMatrix);

	virtual void render(int texturesAdded, const Shader& shader, const FrustumCull& ftc, const glm::mat4& parentMatrix = glm::mat4(1.0f)) override;
	virtual void depthRender(const Shader &shader) override;
	virtual void depthRender(const Shader &shader, const FrustumCull& ftc, const glm::mat4& lightMatrix) override;
};


#include "SceneObjectGeometry.h"
#include "./shading/Shader.h"



SceneObjectGeometry::SceneObjectGeometry(std::shared_ptr<Model> model, const glm::mat4& modelMatrix, float alpha):SceneObject()
{
	this->model = model;	
	this->modelMatrix = modelMatrix;
	this->alpha = alpha;
}


SceneObjectGeometry::SceneObjectGeometry(std::shared_ptr<Model> model, const glm::mat4& modelMatrix):SceneObject()
{
	this->model = model;	
	this->modelMatrix = modelMatrix;
	this->alpha = 1.0f;
}


SceneObjectGeometry::~SceneObjectGeometry()
{
}

void SceneObjectGeometry::setModelMatrix(glm::mat4 modelMatrix)
{
	this->modelMatrix = modelMatrix;
}

void SceneObjectGeometry::render(int texturesAdded, const Shader& shader, const FrustumCull& ftc, const glm::mat4& parentMatrix)
{
	glm::mat4 accumTrasform = transformationMatrix * modelMatrix * parentMatrix;
	shader.use();
	shader.setUniform("model", accumTrasform);
	shader.setUniform("alpha", alpha);	
	model->render(shader, texturesAdded, ftc, accumTrasform);
	SceneObject::render(texturesAdded, shader, ftc, accumTrasform);
}

void SceneObjectGeometry::depthRender(const Shader& shader)
{
	shader.use();
	shader.setUniform("model",  modelMatrix);
	model->depthRender(shader);
	SceneObject::depthRender(shader);
}

void SceneObjectGeometry::depthRender(const Shader& shader, const FrustumCull& ftc, const glm::mat4& lightMatrix) {
	shader.use();
	shader.setUniform("model", modelMatrix);
	model->depthRender(shader, ftc, modelMatrix * lightMatrix);
	SceneObject::depthRender(shader, ftc, modelMatrix * lightMatrix);
}
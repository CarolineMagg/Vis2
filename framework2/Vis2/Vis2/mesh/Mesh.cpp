#include <GL\glew.h> 
#include <iostream>
#include <algorithm>
#include "Mesh.h"
#include "../shading/Shader.h"
#include "../tex/Texture.h"

using namespace std;
RenderMode Mesh::renderMode = RenderMode::NORMAL_TAM;

Mesh::Mesh(vector<Vertex> &vertices, vector<unsigned int> &indices, Material &material, string &name) {
	this->vertices = vertices;
	this->indices = indices;
	this->material = material;
	this->name = name;			
}

void Mesh::render(const Shader& shader, const vector<Texture>& allTextures, const std::vector<TonalArtMap> &allTam ,int occupiedTextures) const {
	if (!visible) return;	

	bool renderTonal = (renderMode != 0);

	shader.use();
	int textureIndex = occupiedTextures;
	int useSpecularMap = 0;
	int useDiffuseMap = 0;
	
	if (material.diffuseMapIndex != -1) {
		glActiveTexture(GL_TEXTURE0 + textureIndex);
		shader.setUniform(UNIFORM_MATERIAL_DIFFUSE, textureIndex);
		glBindTexture(GL_TEXTURE_2D, allTextures[material.diffuseMapIndex].id);
		textureIndex++;
		useDiffuseMap = 1;
		
		glActiveTexture(GL_TEXTURE0 + textureIndex);
		allTam[material.diffuseMapIndex].bindTexture();
		shader.setUniform(UNIFORM_MATERIAL_TONALARTMAP, textureIndex);
		shader.setUniform(UNIFORM_NRTONES, allTam[material.diffuseMapIndex].getNumberOfTones());
		textureIndex++;
		
		
		//shader.setUniform("material.diffuseColor", material.diffuseColor);
	} else {
		shader.setUniform(UNIFORM_MATERIAL_DIFFUSECOLOR, material.diffuseColor);
	}
	if (material.specularMapIndex != -1) {
		glActiveTexture(GL_TEXTURE0 + textureIndex);
		shader.setUniform(UNIFORM_MATERIAL_SPECULAR, textureIndex);
		glBindTexture(GL_TEXTURE_2D, allTextures[material.specularMapIndex].id);
		textureIndex++;
		useSpecularMap = 1;
		//shader.setUniform("material.specularColor", material.specularColor);
	} else {
		shader.setUniform(UNIFORM_MATERIAL_SPECULARCOLOR, material.specularColor);
	}
	if (material.normalMapIndex != -1) {
		glActiveTexture(GL_TEXTURE0 + textureIndex);
		shader.setUniform(UNIFORM_MATERIAL_NORMAL , textureIndex);
		glBindTexture(GL_TEXTURE_2D, allTextures[material.normalMapIndex].id);
		textureIndex++;
	}
	if (material.heightMapIndex != -1) {
		glActiveTexture(GL_TEXTURE0 + textureIndex);
		shader.setUniform(UNIFORM_MATERIAL_HEIGHT , textureIndex);
		glBindTexture(GL_TEXTURE_2D, allTextures[material.heightMapIndex].id);
		textureIndex++;
	}

	shader.setUniform(UNIFORM_MATERIAL_EMISSIVECOLOR, material.emissiveColor);
	shader.setUniform(UNIFORM_MATERIAL_SHININESS, material.shininess);

	glm::ivec4 controls(useDiffuseMap, useSpecularMap, false, renderMode);
	shader.setUniform(UNIFORM_CONTROLS, controls);

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);	
}

void Mesh::render() const {
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);	
}

void Mesh::depthRender() const {
	if (!visible) return;
	
	// draw mesh
	glBindVertexArray(VAO);	
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);	
}


void Mesh::setupMesh() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	glBindVertexArray(0);
	
	calcSphereRadiusAndPosition();
}

void Mesh::calcSphereRadiusAndPosition() {
	glm::vec3 sum;
	for (Vertex& v : vertices) {
		sum += v.Position;
	}

	//std::cout << "sum: " << glm::to_string(sum) << std::endl;

	position.x = sum.x / vertices.size();
	position.y = sum.y / vertices.size();
	position.z = sum.z / vertices.size();
	position.w = 1.0f;

	float maxRadius = 0.0f;
	for (Vertex& v : vertices) {
		maxRadius = std::max(maxRadius, glm::distance(glm::vec3(position), v.Position));
	}
	sphereRadius = maxRadius;
}

const std::string Mesh::UNIFORM_MATERIAL_DIFFUSE = "material.diffuse";
const std::string Mesh::UNIFORM_MATERIAL_TONALARTMAP = "material.tonalArtMap";
const std::string Mesh::UNIFORM_NRTONES = "nrTones";
const std::string Mesh::UNIFORM_MATERIAL_DIFFUSECOLOR = "material.diffuseColor";
const std::string Mesh::UNIFORM_MATERIAL_SPECULAR = "material.specular";
const std::string Mesh::UNIFORM_MATERIAL_SPECULARCOLOR = "material.specularColor";
const std::string Mesh::UNIFORM_MATERIAL_NORMAL = "material.normal";
const std::string Mesh::UNIFORM_MATERIAL_HEIGHT = "material.height";
const std::string Mesh::UNIFORM_MATERIAL_EMISSIVECOLOR = "material.emissiveColor";
const std::string Mesh::UNIFORM_MATERIAL_SHININESS = "material.shininess";
const std::string Mesh::UNIFORM_CONTROLS = "controls";
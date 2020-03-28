#pragma once

#include "RenderStructs.h"
#include <string>
#include <vector>
#include "../shading/Shader.h"
#include "../tex/Texture.h"
#include "../tex/TonalArtMap.h"

enum RenderMode {
	NORMAL,
	TAM,
	NORMAL_TAM
};

class Mesh {
public:	
	static RenderMode renderMode;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	unsigned int VAO;
	Material material;
	std::string name;
	bool visible = true;
		
	float sphereRadius;
	glm::vec4 position;
	void calcSphereRadiusAndPosition();

	Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, Material &material, std::string &name);	
	//render
	void render(const Shader &shader, const std::vector<Texture> &allTextures, const std::vector<TonalArtMap> &allTam, int occupiedTextures) const;
	void render() const;
	void depthRender() const;	

	void setupMesh();

private:	
	unsigned int VBO, EBO;
	bool translated = false;
	glm::vec3 translation;		
	
	std::vector<Material> materialHistory;

	static const std::string UNIFORM_MATERIAL_DIFFUSE;
	static const std::string UNIFORM_MATERIAL_TONALARTMAP;
	static const std::string UNIFORM_NRTONES;
	static const std::string UNIFORM_MATERIAL_DIFFUSECOLOR;
	static const std::string UNIFORM_MATERIAL_SPECULAR;
	static const std::string UNIFORM_MATERIAL_SPECULARCOLOR;
	static const std::string UNIFORM_MATERIAL_NORMAL;
	static const std::string UNIFORM_MATERIAL_HEIGHT;
	static const std::string UNIFORM_MATERIAL_EMISSIVECOLOR;
	static const std::string UNIFORM_MATERIAL_SHININESS;
	static const std::string UNIFORM_CONTROLS;
};
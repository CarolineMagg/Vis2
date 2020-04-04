#pragma once
#include <glm\glm.hpp>
#include <assimp\material.h>
#include "RenderStructs.h"
#include "Mesh.h"
#include "..\tex\TonalArtMap.h"
#include "..\tex\Texture.h"


class FrustumCull;


struct aiScene;
struct aiNode;
struct aiMaterial;
struct aiMesh;


class Model {
public:
	std::string dir;
	std::vector<Mesh> meshes;	
	std::vector<Texture> allTextures;
	std::vector<Material> allMaterials;
	std::vector<TonalArtMap> allTAM;
		
	Model(const std::string &file);		
	void render(const Shader &shader, int occupiedTextures, const FrustumCull& ftc, const glm::mat4& transformMatrix) const;
	void depthRender(const Shader &shader) const;		
	void depthRender(const Shader &shader, const FrustumCull& ftc, const glm::mat4& transformMatrix) const;
	glm::mat4 modelMatrix;

private:		

	void loadModelAndProcessMeshes(std::string const &path);
	void processAssimpNode(const aiNode *node, const aiScene *scene);
	void processMesh(const aiNode *node, const aiMesh *mesh, const aiScene *scene);	
	int loadMaterials(aiMaterial *material, aiTextureType type, std::string typeName);		
};
#include <GL\glew.h> 
#include <iostream>
#include <assimp\scene.h>
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include "Model.h"
#include "../shading/Shader.h"
#include "../render/FrustumCull.h"

using namespace std;

unsigned int Material::nextID = 0;

Model::Model(string const &file) {
	dir = file.substr(0, file.find_last_of('/'));

	loadModelAndProcessMeshes(file);
}

void Model::render(const Shader &shader, int occupiedTextures, const FrustumCull& ftc, const glm::mat4& transformMatrix) const {
	shader.use();
	vector<unsigned int> alphaMeshes;
	unsigned int meshSize = meshes.size();
	unsigned int currentMaterialID = -1;
	
	for (unsigned int i = 0; i < meshSize; i++) {
		const Mesh& mesh = meshes[i];
		if (mesh.material.dFactor < 1.0f) {
			alphaMeshes.push_back(i);
		} else {
			if (ftc.sphereInFrustum(transformMatrix*mesh.position, mesh.sphereRadius) != FrustumCull::OUTSIDE) {
				if (mesh.material.id == currentMaterialID) {
					mesh.render();					
				} else {
					mesh.render(shader, allTextures, allTAM, occupiedTextures);
					currentMaterialID = mesh.material.id;
				}
			}
				
		}
	}
	
	//removed BSP tree - add a way for alpha ordering
	if (alphaMeshes.size() > 0) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (unsigned int i = 0; i < alphaMeshes.size(); i++) {
			const Mesh& m = meshes[alphaMeshes[i]];
			shader.setUniform("alpha", m.material.dFactor);
			m.render(shader, allTextures, allTAM, occupiedTextures);
		}
	}
}

void Model::depthRender(const Shader &shader) const {
	unsigned int meshSize = meshes.size();
	for (unsigned int i = 0; i < meshSize; i++) {
		if (meshes[i].material.dFactor >= 1.0f) {  //ignore alpha stuff
			meshes[i].depthRender();
		}
	}
}

void Model::depthRender(const Shader &shader, const FrustumCull& ftc, const glm::mat4& transformMatrix) const {
	unsigned int meshSize = meshes.size();
	for (unsigned int i = 0; i < meshSize; i++) {
		if (meshes[i].material.dFactor >= 1.0f) {  //ignore alpha stuff
			if (ftc.sphereInFrustum(transformMatrix*meshes[i].position, meshes[i].sphereRadius) != FrustumCull::OUTSIDE)
				meshes[i].depthRender();
		}
	}
}


void Model::loadModelAndProcessMeshes(string const &path) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
		return;
	}
	processAssimpNode(scene->mRootNode, scene);

	//sort meshes by material
	std::cout << "Sorting meshes by Material." << std::endl;
	vector<Mesh> meshes2;
	unsigned int test = meshes.size();
	for (unsigned int i = 0; i < meshes.size(); ++i) {
		meshes2.push_back(meshes[i]);
		for (unsigned int j = i+1; j < meshes.size(); ++j) {
			if (meshes[i].material == meshes[j].material) {
				meshes2.push_back(meshes[j]);
				meshes.erase(meshes.begin() + j);
				j--;
			}
		}
	}
	meshes.clear();
	//meshes.resize(meshes2.size());

	for (auto& m : meshes2) {
		meshes.push_back(m);
	}
	std::cout << "Sorting done." << std::endl;
	
	for (auto& mesh : meshes) {
		mesh.setupMesh();
	}
}

void Model::processAssimpNode(const aiNode *node, const aiScene *scene) {
	// meshes in node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(node, mesh, scene);
	}
	//children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processAssimpNode(node->mChildren[i], scene);
	}
}

void Model::processMesh(const aiNode *node, const aiMesh *mesh, const aiScene *scene) {
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<unsigned int> texturesIndices;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		glm::vec3 vec;
		vec.x = mesh->mVertices[i].x;
		vec.y = mesh->mVertices[i].y;
		vec.z = mesh->mVertices[i].z;
		vertex.Position = vec;
		// normals
		if (mesh->HasNormals()) {
			vec.x = mesh->mNormals[i].x;
			vec.y = mesh->mNormals[i].y;
			vec.z = mesh->mNormals[i].z;
			vertex.Normal = vec;
		}

		if (mesh->mTextureCoords[0]) { //assume only 1 texture and only read first potential value
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		} else {
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}

		if (mesh->HasTangentsAndBitangents()) {
			vec.x = mesh->mTangents[i].x;
			vec.y = mesh->mTangents[i].y;
			vec.z = mesh->mTangents[i].z;
			vertex.Tangent = vec;
			vec.x = mesh->mBitangents[i].x;
			vec.y = mesh->mBitangents[i].y;
			vec.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vec;
		}
		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}
	// process materials
	aiMaterial* assimpMaterial = scene->mMaterials[mesh->mMaterialIndex];
	Material material;
	aiColor3D color;
	float value;

	// 1. diffuse map
	material.diffuseMapIndex = loadMaterials(assimpMaterial, aiTextureType_DIFFUSE, "texture_diffuse");
	// 2. specular map
	material.specularMapIndex = loadMaterials(assimpMaterial, aiTextureType_SPECULAR, "texture_specular");
	// 3. normal map
	material.normalMapIndex = loadMaterials(assimpMaterial, aiTextureType_NORMALS, "texture_normal");
	// 4. height map
	material.heightMapIndex = loadMaterials(assimpMaterial, aiTextureType_HEIGHT, "texture_height");
	// 5. tonal art map
	material.tonalArtMapIndex = loadMaterials(assimpMaterial, aiTextureType_DIFFUSE, "tonalArtMap");

	if (assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS)
		material.ambientColor = glm::vec3(color.r, color.g, color.b);
	if (assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
		material.diffuseColor = glm::vec3(color.r, color.g, color.b);
	if (assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
		material.specularColor = glm::vec3(color.r, color.g, color.b);
	if (assimpMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS)
		material.emissiveColor = glm::vec3(color.r, color.g, color.b);
	if (assimpMaterial->Get(AI_MATKEY_SHININESS, value) == AI_SUCCESS)
		material.shininess = value;
	if (assimpMaterial->Get(AI_MATKEY_OPACITY, value) == AI_SUCCESS) {
		material.dFactor = (float)value;
	} else {
		material.dFactor = 1.0f;
	}

	//material.diffuseColor = glm::vec3(1.0, 1.0, 1.0);

	// don't keep unnecessary structs in memory
	unsigned int i = 0;
	for (i = 0; i < allMaterials.size(); i++) {
		if (allMaterials[i] == material) {
			material = allMaterials[i];
			break;
		}
	}
	if (i == allMaterials.size()) {
		allMaterials.push_back(material);
	}

	string name(mesh->mName.C_Str());

	meshes.emplace_back(vertices, indices, material, name);
}

int Model::loadMaterials(aiMaterial *material, aiTextureType type, string typeName) {
	int texturesCount = material->GetTextureCount(type);
	if (texturesCount > 1) {
#ifdef _DEBUG
		std::cout << "Info: More than 1 texture found for " << type << " - found " << texturesCount << "." << std::endl;
#endif // DEBUG
	}
	if(typeName == "tonalArtMap")
	{
		for (int i = 0; i < texturesCount; i++) {
			aiString str;
			material->GetTexture(type, i, &str);
			string fileName = string(str.C_Str());
			// load tonal art maps ./TEXTURE_NAME/TAM/...
			// Expected directory structure
			if (type == aiTextureType_DIFFUSE)
			{
				//if (allTAM.size() > 0) return 0; // quick load
				for (unsigned int j = 0; j < allTAM.size(); j++) {
					if (std::strcmp(allTAM[j].loaderName.data(), str.C_Str()) == 0) {
						//std::cout << "Info: Re-using texture." << std::endl;
						return j;
					}
				}
				string textureName = fileName.substr(0, fileName.find_first_of('.'));
				allTAM.emplace_back(dir + "\\" + textureName + "\\TAM\\",str.C_Str());
				return allTAM.size()-1;
			}
		}
	}
	else
	{
		for (int i = 0; i < texturesCount; i++) {
			aiString str;
			material->GetTexture(type, i, &str);
			//if (allTextures.size() > 0) return 0;  // quick load
			for (unsigned int j = 0; j < allTextures.size(); j++) {
				if (std::strcmp(allTextures[j].loaderName.data(), str.C_Str()) == 0) {
					//std::cout << "Info: Re-using texture." << std::endl;
					return j;
				}
			}
			std::cout << str.C_Str() << std::endl;
			allTextures.emplace_back(string(str.C_Str()), dir, typeName);
			return allTextures.size() - 1;
		}
	}
	//intentionally in a loop to add multiple textures later

	return -1;
}

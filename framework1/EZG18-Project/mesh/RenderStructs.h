#pragma once

#include <glm\glm.hpp>


struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};

struct Material {
	static unsigned int nextID;
	Material() : shininess(0), diffuseMapIndex(-1), specularMapIndex(-1), normalMapIndex(-1), heightMapIndex(-1) {
		id = nextID;
		nextID++;
	}
	unsigned int id;
	glm::vec3 ambientColor;
	glm::vec3 diffuseColor;
	glm::vec3 specularColor;
	glm::vec3 emissiveColor;
	float shininess;	//shiny
	int diffuseMapIndex;
	int specularMapIndex;
	int normalMapIndex;
	int heightMapIndex;
	int tonalArtMapIndex;

	float dFactor;

	
	int operator==(const Material &rhs) {
		if (ambientColor == rhs.ambientColor &&
			diffuseColor == rhs.diffuseColor &&
			specularColor == rhs.specularColor &&
			emissiveColor == rhs.emissiveColor &&
			shininess == rhs.shininess &&
			diffuseMapIndex == rhs.diffuseMapIndex &&
			specularMapIndex == rhs.specularMapIndex &&
			normalMapIndex == rhs.normalMapIndex &&
			tonalArtMapIndex == rhs.tonalArtMapIndex &&
			dFactor == rhs.dFactor) {
			return 1;
		} else {
			return 0;
		}
	}
};

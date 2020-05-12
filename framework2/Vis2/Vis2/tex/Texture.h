#pragma once
#include <string>
#include "glm/glm.hpp"

class Texture {
public:
	std::string loaderDir;
	std::string loaderName;
	std::string type;
	int width, height, nrChannels;
	unsigned int id;
	GLenum format;

	Texture() = default;
	Texture(std::string fileName, std::string dir, std::string type);

	~Texture();	

	unsigned int loadTexture(std::string path, int width, int height, int nrChannels);
	unsigned int load3DTexture(std::string path, int width, int height, int nrChannels, int beginIndex, int endIndex, std::string fileType, int numLength);
	unsigned int createEmptyTexture(int width, int height, int nrChannels);
	void writeOnTexture(unsigned int x, unsigned int y, float value);
	void writeOnTexture(unsigned int x, unsigned int y, glm::vec3 value);
	void writeOnTexture(unsigned int x, unsigned int y, glm::vec4 value);
};
#pragma once
#include <string>

class Texture {
public:
	std::string loaderDir;
	std::string loaderName;
	std::string type;
	int width, height, nrChannels;
	unsigned int id;

	Texture() = default;
	Texture(std::string fileName, std::string dir, std::string type);

	~Texture();	

	unsigned int loadTexture(std::string path, int width, int height, int nrChannels);
	unsigned int load3DTexture(std::string path, int width, int height, int nrChannels, int beginIndex, int endIndex, std::string fileType, int numLength);
};
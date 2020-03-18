#pragma once
#include <string>

class Texture {
public:
	std::string loaderDir;
	std::string loaderName;
	std::string type;
	int width, height, nrChannels;
	unsigned int id;

	Texture(std::string fileName, std::string dir, std::string type);
	~Texture();	
private:
	unsigned int loadTexture(std::string path, int &width, int &height, int &nrChannels);
};
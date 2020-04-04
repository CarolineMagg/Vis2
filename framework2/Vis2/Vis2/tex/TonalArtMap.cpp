#include <GL\glew.h> 
#include "TonalArtMap.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "stb_image.h"

void TonalArtMap::loadTexture(std::string dir)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D_ARRAY, id);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 10, GL_R8, width, height, tones);

	for (int tone = 0; tone < tones; ++tone)
	{
		std::string path = dir + "tone" + std::to_string(tone) + "/level" + std::to_string(0) + ".png";
		int nrChannels;
		stbi_uc *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			GLenum format;
			if (nrChannels == 3)
				format = GL_RGB;
			else if (nrChannels == 4)
				format = GL_RGBA;
			else if (nrChannels == 1)
				format = GL_RED;
			else
				std::cout << "Unsupported Texture format!" << path << std::endl;

			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, tone, width, height, 1, format, GL_UNSIGNED_BYTE, data);
		}
		else {
			std::cout << "Texture failed to load: " << path << std::endl;
		}
		stbi_image_free(data);
	}
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_LOD_BIAS, -0.1f);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

TonalArtMap::TonalArtMap(std::string dir, std::string fileName)
{
	loaderName = fileName;

	//load texture config
	std::ifstream configFile;
	configFile.open(dir + "config.ini");

	//line string
	std::string line;
	if (configFile.is_open())
	{
		while (std::getline(configFile, line))
		{
			std::istringstream configLine(line);
			std::string key;
			if (std::getline(configLine, key, '=')) {
				std::string value;
				if (std::getline(configLine, value)) {
					if (key == "levels")
					{
						levels = std::stoi(value);
					}
					if (key == "tones")
					{
						tones = std::stoi(value);
					}
					if (key == "width")
					{
						width = std::stoi(value);
					}if (key == "height")
					{
						height = std::stoi(value);
					}
				}
			}
		}
	}
	else
	{
		std::cout << "Coudn't read config file for tonal Art map " << dir << std::endl;
	}
	if (levels != 0 && tones != 0 && width != 0 && height != 0)
	{
		loadTexture(dir);
	}
	else
	{
		std::cout << "levels or tones number missing in config file" << std::endl;
	}
}

TonalArtMap::~TonalArtMap()
{
}

void TonalArtMap::bindTexture() const
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, id);
}

int TonalArtMap::getNumberOfTones() const
{
	return tones;
}

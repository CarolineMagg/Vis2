#include <GL\glew.h> 
#include <iostream>
#include <vector>
#include "Texture.h"
#include "glm/gtc/type_ptr.hpp"


#ifndef  STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

using namespace std;

Texture::~Texture() {
}

Texture::Texture(string fileName, string dir, string type) {
	loaderDir = dir;
	loaderName = fileName;
	
	id = loadTexture(loaderDir + "/" + loaderName, width, height, nrChannels);
}


unsigned int Texture::loadTexture(string path, int width, int height, int nrChannels) {

	unsigned int id;
	glGenTextures(1, &id);

	//stbi_set_flip_vertically_on_load(true);
	stbi_uc *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;
		else
			cout << "Unsupported Texture format!" << path << endl;

		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	} else {
		cout << "Texture failed to load: " << path << endl;
	}
	stbi_image_free(data);

	return id;
}

unsigned int Texture::load3DTexture(std::string path, int width, int height, int nrChannels, int beginIndex, int endIndex, std::string fileType, int numLength)
{
	std::cout << "Load " << path << std::endl;
	unsigned int id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_3D, id);	
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_ANISOTROPY, 8.0);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_LOD_BIAS, -1.0);
		
	GLenum sizedFormat;
	
	for (int i = beginIndex; i < endIndex + 1; i++)
	{
		std::string indexS = std::to_string(i);
		while (indexS.length() < numLength) {
			indexS = "0" + indexS;
		}
		std::string path_img = path + indexS + fileType;
		//std::cout << path_img << endl;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* data = stbi_load(path_img.c_str(), &width, &height, &nrChannels, 0);

		if (data) {

			if (i == beginIndex) {	

				if (nrChannels == 1) {
					format = GL_RED;
					sizedFormat = GL_R8;
				}					
				else if (nrChannels == 3) {
					format = GL_RGB;
					sizedFormat = GL_RGB8;
				}
				else if (nrChannels == 4) {
					format = GL_RGBA;
					sizedFormat = GL_RGBA8;
				} else
					cout << "Unsupported Texture format!" << path << endl;

				glTexStorage3D(GL_TEXTURE_3D, 1, sizedFormat, width, height, endIndex - beginIndex + 1);
			}

			glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i - beginIndex, width, height, 1, format, GL_UNSIGNED_BYTE, data);			
		}
		else {
			cout << "Texture failed to load: " << path << endl;
		}
		stbi_image_free(data);
	}
	glGenerateMipmap(GL_TEXTURE_3D);

	glBindTexture(GL_TEXTURE_3D, 0);
	return id;
}

unsigned int Texture::createEmptyTexture(int width, int height, int nrChannels, int tSizedFormat)
{
	this->width = width;
	this->height = height;
	this->nrChannels = nrChannels;
	
	GLenum sizedFormat;
	if (nrChannels == 1) {
		format = GL_RED;
		sizedFormat = tSizedFormat == -1 ? GL_R8 : tSizedFormat;
	}
	else if (nrChannels == 3) {
		format = GL_RGB;
		sizedFormat = tSizedFormat == -1 ? GL_RGB8 : tSizedFormat;
	}
	else if (nrChannels == 4) {
		format = GL_RGBA;
		sizedFormat = tSizedFormat == -1 ? GL_RGBA8 : tSizedFormat;
	}
	else
		cout << "Unsupported Texture format!" << endl;
		
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexStorage2D(GL_TEXTURE_2D, 1, sizedFormat, width, height);	


	glBindTexture(GL_TEXTURE_2D, 0);	
	return id;
}

unsigned int Texture::createEmptyCubeTexture(int width, int height, int nrChannels, int tSizedFormat)
{
	this->width = width;
	this->height = height;
	this->nrChannels = nrChannels;

	GLenum sizedFormat;
	if (nrChannels == 1) {
		format = GL_RED;
		sizedFormat = tSizedFormat == -1 ? GL_R8 : tSizedFormat;
	}
	else if (nrChannels == 3) {
		format = GL_RGB;
		sizedFormat = tSizedFormat == -1 ? GL_RGB8 : tSizedFormat;
	}
	else if (nrChannels == 4) {
		format = GL_RGBA;
		sizedFormat = tSizedFormat == -1 ? GL_RGBA8 : tSizedFormat;
	}
	else
		cout << "Unsupported Texture format!" << endl;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, sizedFormat, width, height);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return id;
}

void Texture::writeOnTexture(unsigned int x, unsigned int y, const float value)
{
	glBindTexture(GL_TEXTURE_2D, id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, format, GL_UNSIGNED_BYTE, &value);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::writeOnTexture(unsigned int x, unsigned int y, const glm::vec3 value)
{
	glBindTexture(GL_TEXTURE_2D, id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, format, GL_UNSIGNED_BYTE, &value[0]);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::writeOnTexture(unsigned width, unsigned int height, unsigned char* data)
{
	glBindTexture(GL_TEXTURE_2D, id);
	//glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//stbi_set_flip_vertically_on_load(doFlip);
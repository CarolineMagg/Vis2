#include <gl3w.h>
#include <iostream>
#include "Texture.h"


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


unsigned int Texture::loadTexture(string path, int &width, int &height, int &nrChannels) {

	unsigned int id;
	glGenTextures(1, &id);

	stbi_uc *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		GLenum format;
		if (nrChannels == 3)
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

//stbi_set_flip_vertically_on_load(doFlip);
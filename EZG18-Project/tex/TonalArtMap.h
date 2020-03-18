#pragma once
#include <gl3w.h>
#include <string>

class TonalArtMap
{
private:
	GLuint id;
	void loadTexture(std::string dir);
	int levels=0;
	int tones=0;
	int width, height = 0;
public:
	TonalArtMap(std::string dir, std::string fileName);
	~TonalArtMap();

	void bindTexture() const;
	int getNumberOfTones() const;
	std::string loaderName;
};


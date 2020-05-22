#pragma once
#include "spline.h"
#include <vector>
#include "../tex/Texture.h"

class TransferTableBuilder {
public:
	TransferTableBuilder() = default;
	TransferTableBuilder(glm::vec3 newColor1, glm::vec3 newColor2, glm::vec3 newColor3, glm::vec3 newColor4, glm::vec4 newPosition);

	Texture colorTexture;

	static unsigned int getColorAlphaTransferTextureStatic();
	unsigned int getColorAlphaTransferTexture();
	void setColor1(glm::vec3 newColor);
	void setColor2(glm::vec3 newColor);
	void setColor3(glm::vec3 newColor);
	void setColor4(glm::vec3 newColor);
	void setPosition(glm::vec4 newPos);
	void setSplines();
	unsigned int getTransfer();
	void setColorsPos(glm::vec3 newColor1, glm::vec3 newColor2, glm::vec3 newColor3, glm::vec3 newColor4, glm::vec4 newPosition);


private:

	glm::vec3 color1;
	glm::vec3 color2;
	glm::vec3 color3;
	glm::vec3 color4;
	glm::vec4 position;
	tk::spline r, g, b, a;

};
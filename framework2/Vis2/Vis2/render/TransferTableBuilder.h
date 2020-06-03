#pragma once
#include "spline.h"
#include <vector>
#include "../tex/Texture.h"
#include <iostream>

class TransferTableBuilder {
public:
	TransferTableBuilder() = default;
	TransferTableBuilder(glm::vec4 *newColor);
	TransferTableBuilder(glm::vec4 newColor1, glm::vec4 newColor2, glm::vec4 newColor3, glm::vec4 newColor4, glm::vec4 newPosition);

	Texture colorTexture;

	static unsigned int getColorAlphaTransferTextureStatic();
	unsigned int getColorAlphaTransferTexture();
	void setColor1(glm::vec4 newColor);
	void setColor2(glm::vec4 newColor);
	void setColor3(glm::vec4 newColor);
	void setColor4(glm::vec4 newColor);
	void setPosition(glm::vec4 newPos);
	void setSplines();
	unsigned int getTransfer();
	void setColorsPos(glm::vec4 *newColors);
	void setColorsPos(glm::vec4 newColor1, glm::vec4 newColor2, glm::vec4 newColor3, glm::vec4 newColor4, glm::vec4 newPosition);


private:

	glm::vec4 color1;
	glm::vec4 color2;
	glm::vec4 color3;
	glm::vec4 color4;
	glm::vec4 position;
	tk::spline r, g, b, a;

};
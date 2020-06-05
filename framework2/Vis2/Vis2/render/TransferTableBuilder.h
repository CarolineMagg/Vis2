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
	unsigned int resetColorAlphaTransferTexture();
	void setSplines();
	unsigned int getTransfer();

	glm::vec4 color1;
	glm::vec4 color2;
	glm::vec4 color3;
	glm::vec4 color4;
	glm::vec4 position;

private:

	glm::vec4 color1Reset;
	glm::vec4 color2Reset;
	glm::vec4 color3Reset;
	glm::vec4 color4Reset;
	glm::vec4 positionReset;
	tk::spline r, g, b, a;

};
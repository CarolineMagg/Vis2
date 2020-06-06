#pragma once
#include "spline.h"
#include <vector>
#include "../tex/Texture.h"
#include <iostream>

class TransferTableBuilder {
public:
	TransferTableBuilder() = default;
	TransferTableBuilder(int id);
	Texture colorTexture;

	unsigned int getColorAlphaTransferTexture();
	unsigned int resetColorAlphaTransferTexture();
	
	unsigned int getTransfer();
	
	std::vector<double> rPos;
	std::vector<double> rCol;
	std::vector<double> gPos;
	std::vector<double> gCol;
	std::vector<double> bPos;
	std::vector<double> bCol;
	std::vector<double> aPos;
	std::vector<double> aCol;

	glm::vec2 pointNumbers[4];

private:

	void initColorAlphaTransferTexture();

	void setSplines();
	tk::spline r, g, b, a;

};
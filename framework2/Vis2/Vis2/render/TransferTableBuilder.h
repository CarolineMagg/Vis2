#pragma once
#include "spline.h"
#include <vector>
#include <functional> 
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

	glm::vec3 pointNumbers[4];

private:

	void initColorAlphaTransferTexture(int id);
	void checkValuesColor(std::vector<double> &color);
	void checkValuesPosition(std::vector<double> &position);

	void setSplines();
	tk::spline r, g, b, a;
	int id;

};
#pragma once
#include "spline.h"
#include <vector>
#include <functional> 
#include "../tex/Texture.h"
#include <iostream>

enum TransferType {
	COLOR,
	MEDIUM
};

class TransferTableBuilder {
public:
	TransferTableBuilder() = delete;
	TransferTableBuilder(TransferType type);
	Texture colorTexture;

	void initResources();

	unsigned int getColorAlphaTransferTexture();
	unsigned int resetColorAlphaTransferTexture();
	
	unsigned int getTransferTexture();
	
	std::vector<double> rPos;
	std::vector<double> rCol;
	std::vector<double> gPos;
	std::vector<double> gCol;
	std::vector<double> bPos;
	std::vector<double> bCol;
	std::vector<double> aPos;
	std::vector<double> aCol;

	glm::vec3 pointNumbers[4];

	int getType() { return id;  }

private:

	void initColorAlphaTransferTexture(TransferType id);
	void checkValuesColor(std::vector<double> &color);
	void checkValuesPosition(std::vector<double> &position);

	void setSplines();
	tk::spline r, g, b, a;
	TransferType id;

};
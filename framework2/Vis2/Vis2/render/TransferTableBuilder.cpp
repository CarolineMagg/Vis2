
#include "TransferTableBuilder.h"
TransferTableBuilder::TransferTableBuilder(int id)
{
	std::cout << "Init transfer function " << id << std::endl;
	colorTexture.createEmptyTexture(256, 256, 4);
	this->id = id;
	initColorAlphaTransferTexture(id);
	getColorAlphaTransferTexture();
}

void TransferTableBuilder::initColorAlphaTransferTexture(int id) 
{
	rPos = { 0.0, 0.5, 0.8, 1.0 };
	rCol = { 0, 0, 100, 255 };

	gPos = { 0.0, 0.2, 0.6, 0.8, 1.0 };
	gCol = { 0, 100, 35, 120, 0 };

	bPos = { 0.0, 0.5, 1.0 };
	bCol = { 0,  100, 0 };

	aPos = { 0.0, 0.6, 1.0 };
	aCol = { 0.0,  0.05 * 255.0, 0.4 * 255.0 };

	pointNumbers[0] = glm::vec3(1,rCol.size(),id);
	pointNumbers[1] = glm::vec3(2, gCol.size(),id);
	pointNumbers[2] = glm::vec3(3, bCol.size(),id);
	pointNumbers[3] = glm::vec3(4, aCol.size(),id);

}

unsigned int TransferTableBuilder::getColorAlphaTransferTexture()
{
	checkValuesColor(rCol);
	checkValuesColor(gCol);
	checkValuesColor(bCol);
	checkValuesColor(aCol);
	checkValuesPosition(rPos);
	checkValuesPosition(gPos);
	checkValuesPosition(bPos);
	checkValuesPosition(aPos);

	setSplines();

	pointNumbers[0] = glm::vec3(1, rCol.size(), id);
	pointNumbers[1] = glm::vec3(2, gCol.size(), id);;
	pointNumbers[2] = glm::vec3(3, bCol.size(), id);
	pointNumbers[3] = glm::vec3(4, aCol.size(), id);

	double x = 1.0;
	unsigned __int8 imageData[256 * 256 * 4];
	unsigned int nextInd = 0;
	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 256; j++)
		{

			//colorTexture.writeOnTexture(i, j, (glm::vec3(r(i), g(i), b(i)) + glm::vec3(r(j), g(j), b(j))) / 2.0f);
			unsigned int ri = r((double)i / 255.0);
			unsigned int rj = r((double)j / 255.0);
			unsigned int gi = g((double)i / 255.0);
			unsigned int gj = g((double)j / 255.0);
			unsigned int bi = b((double)i / 255.0);
			unsigned int bj = b((double)j / 255.0);
			unsigned int ai = a((double)i / 255.0);
			unsigned int aj = a((double)j / 255.0);
			glm::ivec4 colorResults = (glm::ivec4(ri, gi, bi, 255) + glm::ivec4(rj, gj, bj, 255)) / 2;

			imageData[nextInd++] = (ri + rj) / 2;
			imageData[nextInd++] = (gi + gj) / 2;
			imageData[nextInd++] = (bi + bj) / 2;
			imageData[nextInd++] = (ai + aj) / 2;
		}
	}

	colorTexture.writeOnTexture(256, 256, imageData);

	return colorTexture.id;
}

unsigned int TransferTableBuilder::resetColorAlphaTransferTexture()
{
	colorTexture.createEmptyTexture(256, 256, 4);
	initColorAlphaTransferTexture(id);
	return getColorAlphaTransferTexture();
}

void TransferTableBuilder::setSplines()
{
	r.set_points(rPos, rCol);
	g.set_points(gPos, gCol);
	b.set_points(bPos, bCol);
	a.set_points(aPos, aCol);
}

unsigned int TransferTableBuilder::getTransfer()
{
	return colorTexture.id;
}

void TransferTableBuilder::checkValuesColor(std::vector<double> &color) {
	for (std::vector<int>::size_type i = 0; i != color.size(); i++) {
		if (color[i] > 255) {
			color[i] = 255;
		}
		if (color[i] < 0) {
			color[i] = 0;
		}
	}
}

void TransferTableBuilder::checkValuesPosition(std::vector<double> &position) {
	for (std::vector<int>::size_type i = 0; i != position.size(); i++) {
		if (position[i] > 1) {
			position[i] = 1;
		}
		if (position[i] < 0) {
			position[i] = 0;
		}
	}
}

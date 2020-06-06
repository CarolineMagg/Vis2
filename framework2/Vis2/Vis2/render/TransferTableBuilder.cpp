
#include "TransferTableBuilder.h"
TransferTableBuilder::TransferTableBuilder(int id)
{
	std::cout << "Init transfer function " << id << std::endl;
	colorTexture.createEmptyTexture(256, 256, 4);
	initColorAlphaTransferTexture();
	getColorAlphaTransferTexture();
}

void TransferTableBuilder::initColorAlphaTransferTexture() 
{
	rPos = { 0.0, 0.5, 0.8, 1.0 };
	rCol = { 0, 0, 100, 255 };

	gPos = { 0.0, 0.2, 0.6, 0.8, 1.0 };
	gCol = { 0, 100, 35, 120, 0 };

	bPos = { 0.0, 0.5, 1.0 };
	bCol = { 0,  100, 0 };

	aPos = { 0.0, 0.6, 1.0 };
	aCol = { 0.0,  0.05 * 255.0, 0.4 * 255.0 };

	pointNumbers[0] = glm::vec2(1,rCol.size());
	pointNumbers[1] = glm::vec2(2, gCol.size());
	pointNumbers[2] = glm::vec2(3, bCol.size());
	pointNumbers[3] = glm::vec2(4, aCol.size());

}

unsigned int TransferTableBuilder::getColorAlphaTransferTexture()
{
	setSplines();

	pointNumbers[0] = glm::vec2(1, rCol.size());
	pointNumbers[1] = glm::vec2(2, gCol.size());
	pointNumbers[2] = glm::vec2(3, bCol.size());
	pointNumbers[3] = glm::vec2(4, aCol.size());

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
	initColorAlphaTransferTexture();
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

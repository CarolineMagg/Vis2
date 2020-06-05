
#include "TransferTableBuilder.h"

TransferTableBuilder::TransferTableBuilder(glm::vec4 newColor1, glm::vec4 newColor2, glm::vec4 newColor3, glm::vec4 newColor4, glm::vec4 newPosition)
{
	std::cout << "Init transfer function " << std::endl;
	colorTexture.createEmptyTexture(256, 256, 4);
	color1 = newColor1;
	color2 = newColor2;
	color3 = newColor3;
	color4 = newColor4;
	position = newPosition;
	color1Reset = color1;
	color2Reset = color2;
	color3Reset = color3;
	color4Reset = color4;
	positionReset = position;
	getColorAlphaTransferTexture();
}

TransferTableBuilder::TransferTableBuilder(glm::vec4 *newColor)
{
	std::cout << "Init transfer function " << std::endl;
	colorTexture.createEmptyTexture(256, 256, 4);
	color1 = newColor[0];
	color2 = newColor[1];
	color3 = newColor[2];
	color4 = newColor[3];
	position = newColor[4];
	color1Reset = color1;
	color2Reset = color2;
	color3Reset = color3;
	color4Reset = color4;
	positionReset = position;
	getColorAlphaTransferTexture();
}


unsigned int TransferTableBuilder::getColorAlphaTransferTextureStatic()
{
	Texture colorTexture;
	colorTexture.createEmptyTexture(256, 256, 4);

	std::vector<double> rX{ 0.0, 0.5, 0.8, 1.0 };
	std::vector<double> rY{ 0, 0, 100, 255 };

	std::vector<double> gX{ 0.0, 0.2, 0.6, 0.8, 1.0 };
	std::vector<double> gY{ 0, 100, 35, 120, 0 };

	std::vector<double> bX{ 0.0, 0.5, 1.0 };
	std::vector<double> bY{ 0,  100, 0 };

	std::vector<double> aX{ 0.0, 0.6, 1.0 };
	std::vector<double> aY{ 0.0,  0.05 * 255.0, 0.4 * 255.0 };

	tk::spline r, g, b, a;
	r.set_points(rX, rY);
	g.set_points(gX, gY);
	b.set_points(bX, bY);
	a.set_points(aX, aY);

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

	//glGenerateTextureMipmap(colorTransfer.id);

	return colorTexture.id;
}

unsigned int TransferTableBuilder::getColorAlphaTransferTexture()
{
	setSplines();

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
	color1 = color1Reset;
	color2 = color2Reset;
	color3 = color3Reset;
	color4 = color4Reset;
	position = positionReset;
	return getColorAlphaTransferTexture();
}

void TransferTableBuilder::setSplines()
{
	std::vector<double> rX{ position.x, position.y, position.z, position.w };
	std::vector<double> rY{ color1.x*255.0, color2.x*255.0, color3.x*255.0, color4.x*255.0 };

	std::vector<double> gX{ position.x, position.y, position.z, position.w };
	std::vector<double> gY{ color1.y*255.0, color2.y*255.0, color3.y*255.0, color4.y*255.0 };

	std::vector<double> bX{ position.x, position.y, position.z, position.w };
	std::vector<double> bY{ color1.z*255.0, color2.z*255.0, color3.z*255.0, color4.z*255.0 };

	std::vector<double> aX{ position.x, position.y, position.z, position.w };
	std::vector<double> aY{ color1.w*255.0, color2.w*255.0, color3.w*255.0, color4.w*255.0 };

	r.set_points(rX, rY);
	g.set_points(gX, gY);
	b.set_points(bX, bY);
	a.set_points(aX, aY);
}

unsigned int TransferTableBuilder::getTransfer()
{
	return colorTexture.id;
}

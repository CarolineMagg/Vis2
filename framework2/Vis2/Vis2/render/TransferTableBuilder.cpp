#include "../tex/Texture.h"
#include "TransferTableBuilder.h"
#include "spline.h"
#include <vector>
unsigned int TransferTableBuilder::getColorAlphaTransferTexture()
{
	Texture colorTransfer;
	colorTransfer.createEmptyTexture(256, 256, 4);

	std::vector<double> rX{ 0.0, 0.5, 0.8, 1.0 };
	std::vector<double> rY{ 0, 0, 0, 255 };

	std::vector<double> gX{ 0.0, 0.2, 0.6, 0.8, 1.0 };
	std::vector<double> gY{ 0, 100, 35, 120, 255 };

	std::vector<double> bX{ 0.0, 0.5, 1.0 };
	std::vector<double> bY{ 0,  200, 255 };

	std::vector<double> aX{ 0.0, 0.6, 1.0 };
	std::vector<double> aY{ 0,  0.3 * 255.0, 1.0 * 255.0 };

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

			//colorTransfer.writeOnTexture(i, j, (glm::vec3(r(i), g(i), b(i)) + glm::vec3(r(j), g(j), b(j))) / 2.0f);
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

	colorTransfer.writeOnTexture(256, 256, imageData);

	return colorTransfer.id;
}
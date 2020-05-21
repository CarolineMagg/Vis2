#pragma once


class TransferTableBuilder {
public:
	static unsigned int getColorAlphaTransferTexture();
	static unsigned int getColorAlphaTransferTexture(glm::vec3 color1, glm::vec3 color2, glm::vec3 color3, glm::vec3 color4, glm::vec4 position);
};
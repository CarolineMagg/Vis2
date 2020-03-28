#pragma once
#include <gl3w.h>
#include <vector>
class FBO {
public:

	FBO(bool useTextureColor, bool useTextureDepth, bool multiSample, unsigned int width, unsigned int height, bool addSecondColorBuffer = false);
	FBO(unsigned int nrColorTexture, bool useDepthTexture, bool useMultisampling, unsigned int width, unsigned int height);
	~FBO();

	static void copyFBO(const FBO& from, const FBO& to, unsigned int bufferMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static void copyFBO(const FBO& from, unsigned int to, unsigned int bufferMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static void copyFBO(unsigned int from, const FBO& to, unsigned int bufferMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const unsigned int getColorTexture() const { return colorTextures.at(0); }
	const unsigned int getColorTexture(unsigned int i) const { return colorTextures.at(i); }
	const unsigned int getSecondaryColorTexture() const { return colorTextures.at(1); }
	const unsigned int getDepthTexture() const { return depthTexture; }
	const bool isMultiSample() const { return multiSample; };

	void setActive();
	void changeSize(unsigned int width, unsigned int height);

	static unsigned int MULTI_SAMPLE_COUNT;
private:
	unsigned int fbo;
	unsigned int width;
	unsigned int height;
	unsigned int colorTexture; //not needed
	unsigned int colorTexture2; //not needed
	unsigned int depthTexture;



	bool useTextureDepth;
	bool useTextureColor;
	std::vector<unsigned int> colorTextures;
	std::vector<unsigned int> multiSampleTextures;
	bool hasSecondaryColorBuffer;
	bool multiSample;

	void createFrameBuffers(unsigned int nrColorTexture, bool useDepthTexture, bool useMultisampling, unsigned int width, unsigned int height);

	
};


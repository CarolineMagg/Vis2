#pragma once
#include <GL\glew.h> 
#include <GLFW\glfw3.h>
#include <vector>
class FBO {
public:

	FBO(unsigned int nrColorTexture, bool useDepthTexture, bool useMultisampling, unsigned int width, unsigned int height, int layers = 1);
	~FBO();

	static void copyFBO(const FBO& from, const FBO& to, unsigned int bufferMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static void copyFBO(const FBO& from, unsigned int to, unsigned int bufferMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static void copyFBO(unsigned int from, const FBO& to, unsigned int bufferMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const unsigned int getColorTexture() const { return colorTextures.at(0); }
	const unsigned int getColorTexture(unsigned int i) const { return colorTextures.at(i); }
	const unsigned int getSecondaryColorTexture() const { return colorTextures.at(1); }
	const unsigned int getDepthTexture() const { return depthTexture; }
	const bool isMultiSample() const { return multiSample; };

	void setActive(bool doClear = true);
	void changeSize(unsigned int width, unsigned int height);
		
	void bindColorTextureAsImageUnit(unsigned int index, unsigned int bindIndex = 0, int option = GL_WRITE_ONLY);
	void bindAllColorTexturesAsImageUnits();

	static unsigned int MULTI_SAMPLE_COUNT;
private:
	unsigned int fbo;
	unsigned int width;
	unsigned int height;	
	unsigned int depthTexture;

	bool useTextureDepth;
	bool useTextureColor;
	std::vector<unsigned int> colorTextures;
	std::vector<unsigned int> multiSampleTextures;	
	bool multiSample;
	int layers = 1;

	void createFrameBuffers(unsigned int nrColorTexture, bool useDepthTexture, bool useMultisampling, unsigned int width, unsigned int height, int layers);	
};


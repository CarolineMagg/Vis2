#include "FBO.h"
#include <iostream>

unsigned int FBO::MULTI_SAMPLE_COUNT = 16;


FBO::FBO(unsigned int nrColorTexture, bool useDepthTexture, bool useDepthMultisample, unsigned int width, unsigned int height, int layers) 
	:multiSample(useDepthMultisample), width(width), height(height)
{
	createFrameBuffers(nrColorTexture, useDepthTexture, useDepthMultisample, width, height, layers);
}


FBO::~FBO() {
	//delete stuff here //TODO

}

void FBO::setActive(bool doClear) {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, width, height);
	if (doClear)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FBO::changeSize(unsigned int width, unsigned int height)
{
}

void FBO::createFrameBuffers(unsigned int nrColorTexture, bool useDepthTexture, bool useMultisampling, unsigned int width, unsigned int height, int layers)
{
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	this->layers = layers;
	if (nrColorTexture > 0) {
		colorTextures = std::vector<unsigned int>(nrColorTexture);
		if (useMultisampling)
		{			
			glGenTextures(nrColorTexture, colorTextures.data());
			for (unsigned int i = 0; i<nrColorTexture; ++i)
			{
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorTextures.at(i));
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MULTI_SAMPLE_COUNT, GL_RGBA, width, height, GL_TRUE);
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, colorTextures.at(i), 0);
			}
		}
		else
		{
			glGenTextures(nrColorTexture, colorTextures.data());
			
			for (unsigned int i = 0; i < nrColorTexture; ++i)
			{
				
				if (layers > 1)
				{
					glBindTexture(GL_TEXTURE_2D_ARRAY, colorTextures.at(i));
					glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA32F, width, height, layers);
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colorTextures.at(i), 0);

					float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
					glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, colorTextures.at(i));
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glBindTexture(GL_TEXTURE_2D, 0);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorTextures.at(i), 0);
				}
				
				
				
			}
		}
	}
	//depth Texture
	if (useDepthTexture) {
		glGenTextures(1, &depthTexture);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		if (useMultisampling) {
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTexture);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MULTI_SAMPLE_COUNT, GL_DEPTH_COMPONENT, width, height, GL_TRUE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthTexture, 0);
		}
		else {
			if (layers > 1)
			{
				glBindTexture(GL_TEXTURE_2D_ARRAY, depthTexture);
				glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT16, width, height, layers);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, depthTexture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
				glBindTexture(GL_TEXTURE_2D, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
			}			
		}
	}
	else {
		unsigned int rb;
		glGenRenderbuffers(1, &rb);
		glBindRenderbuffer(GL_RENDERBUFFER, rb);
		if (multiSample) {
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, MULTI_SAMPLE_COUNT, GL_DEPTH_COMPONENT32, width, height);
		}
		else {
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
		}
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);
	}
	std::vector<GLenum> attachments;
	for (unsigned int i = 0; i < nrColorTexture; ++i) {
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	glNamedFramebufferDrawBuffers(fbo, nrColorTexture, attachments.data());

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR - FRAMEBUFFER is not completed!" << std::endl;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::copyFBO(const FBO& from, const FBO& to, unsigned int bufferMask) {

	/*if (from.hasSecondaryColorBuffer && to.hasSecondaryColorBuffer) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, from.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to.fbo);
		glNamedFramebufferReadBuffer(from.fbo, GL_COLOR_ATTACHMENT1);
		glNamedFramebufferDrawBuffer(to.fbo, GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, from.width, from.height, 0, 0, to.width, to.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glNamedFramebufferReadBuffer(from.fbo, GL_COLOR_ATTACHMENT0);
		glNamedFramebufferDrawBuffer(to.fbo, GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, from.width, from.height, 0, 0, to.width, to.height, bufferMask, GL_NEAREST);
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glNamedFramebufferDrawBuffers(to.fbo, 2, attachments);
	}
	else {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, from.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to.fbo);
		glBlitFramebuffer(0, 0, from.width, from.height, 0, 0, to.width, to.height, bufferMask, GL_NEAREST);
	}
	*/
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::copyFBO(const FBO& from, unsigned int to, unsigned int bufferMask) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, from.fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to);
	glBlitFramebuffer(0, 0, from.width, from.height, 0, 0, from.width, from.height, bufferMask, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::copyFBO(unsigned int from, const FBO& to, unsigned int bufferMask) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, from);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to.fbo);
	glBlitFramebuffer(0, 0, to.width, to.height, 0, 0, to.width, to.height, bufferMask, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::bindColorTextureAsImageUnit(unsigned int index, unsigned int bindIndex)
{
	glBindImageTexture(bindIndex, getColorTexture(index), 0, layers > 1 ? GL_TRUE : GL_FALSE, layers, GL_WRITE_ONLY, GL_RGBA32F);
}

void FBO::bindAllColorTexturesAsImageUnits()
{
	int index = 0;
	for (int i : colorTextures)
		glBindImageTexture(index++, i, 0, layers > 1 ? GL_TRUE : GL_FALSE, layers, GL_WRITE_ONLY, GL_RGBA32F);
}
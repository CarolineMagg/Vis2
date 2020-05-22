#include <GL\glew.h> 
#include <GLFW\glfw3.h>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\string_cast.hpp>

#include "camera\Camera.h"
#include "tex/Texture.h"
#include "mesh/Cube.h"
#include "render/FBO.h"
#include "AntTweakBar.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <locale>
#include <algorithm>
#include <memory>

#include "shading/Shader.h"
#include "render/TransferTableBuilder.h"


unsigned int WIDTH_DEFAULT = 512;
unsigned int HEIGHT_DEFAULT = 512;

#define EXIT_WITH_ERROR(err) \
	std::cout << "ERROR: " << err << std::endl; \
	system("PAUSE"); \
	return EXIT_FAILURE;

// set parameters
float fov = 85.0f;
int fps = 10;
float frameDeltaTime = 0;
bool isFullScreen = false;
bool wireframe = false;
float brightness = 0.00f;
bool allowMouseMove = false;

// create objects
GLFWwindow* window;
Camera* cameraPointer;
float bgColor[] = { 0.0f, 0.0f, 0.0f };
typedef enum { HEAD, Option2, } MESH_TYPE;
MESH_TYPE m_currentMesh = HEAD;
TwBar *bar;
int volumeTexture;
int volume1;
unsigned int colorTransfer;
glm::vec3 oldColor1 = glm::vec3(0.0, 0.0, 0.0);
glm::vec3 oldColor2 = glm::vec3(0.0, 0.33, 0.47);
glm::vec3 oldColor3 = glm::vec3(0.2, 0.13, 0.82);
glm::vec3 oldColor4 = glm::vec3(1.0, 1.0, 1.0);
glm::vec4 oldPos = glm::vec4(0.0, 0.3, 0.6, 1.0);
TransferTableBuilder transferFunct;

void renderQuad();

// method init
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processMouseInput(GLFWwindow* window, double xpos, double ypos);
void processScrollInput(GLFWwindow* window, double xoffset, double yoffset);
void processMouseButton(GLFWwindow* window, int button, int action, int mods);
void processKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods);
int ChangeVolume(MESH_TYPE currentMesh);
void ChangeTransfer(glm::vec3 newColor1, glm::vec3 newColor2, glm::vec3 newColor3, glm::vec3 newColor4, glm::vec4 newPosition);
void TW_CALL ResetCamera(void *clientData);

GLFWwindow* setupAndConfigWindow();
static void APIENTRY DebugCallbackDefault(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);

TwStructMember Vector3fMembers[] = {
	{ "x", TW_TYPE_FLOAT, offsetof(glm::vec3, x), "" },
	{ "y", TW_TYPE_FLOAT, offsetof(glm::vec3, y), "" },
	{ "z", TW_TYPE_FLOAT, offsetof(glm::vec3, z), "" }
};

int main()
{		
	if (setupAndConfigWindow() == nullptr)
	{
		std::cout << "FAILED TO SET WINDOW UP" << std::endl;
		return -1;
	}

	// setup camera
	Camera camera(fov, (float)(WIDTH_DEFAULT) / HEIGHT_DEFAULT, 0.001f, 120.0f);
	cameraPointer = &camera;

	// this is for drawing random squares
#ifdef _DEBUG
	Shader debugDrawShader("debug_texturedraw_vs.txt", "debug_texturedraw_fs.txt");
#endif

	// init shaders
	Shader basicRender("basic_render_vs.txt", "basic_render_fs.txt");
	Shader volumeRender("vol_vs.txt", "vol_fs.txt");
	Shader mainVolumeRender("vol_vs.txt", "volume.frag", "volume.geom");
	Shader debugDrawShader2("vol_vs.txt", "debug_texturedraw_fs.txt", "debug_texturedraw.geom");
	Shader computeShader("volume.comp");

	// setup scene
	printf("Loading assets...\n");

	glm::vec3 color1 = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 color2 = glm::vec3(0.0, 0.33, 0.47);
	glm::vec3 color3 = glm::vec3(0.2, 0.13, 0.82);
	glm::vec3 color4 = glm::vec3(1.0, 1.0, 1.0);
	glm::vec4 pos = glm::vec4(0.0, 0.3, 0.6, 1.0);
	//colorTransfer = TransferTableBuilder::getColorAlphaTransferTexture(color1, color2, color3, color4, pos);	
	transferFunct = TransferTableBuilder(color1, color2, color3, color4, pos);
	colorTransfer = transferFunct.getTransfer();
	volume1 = (Texture()).load3DTexture("assets/images/volumeData/Choloepis_hoffmani/PNG/CHOL", 512, 512, 1, 1, 441, ".png", 4);
	ChangeVolume(m_currentMesh);

	glm::vec3 lightVSPos = glm::vec3(0, 0, 0);
	glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0) * 3.0f;
	glm::vec3 voxelSize = glm::vec3(1.0f, 1.0f, 0.2409f / 0.1284f);
	
	// main FBO: 6 buffers and 1 debug buffer
	FBO ilFBO(7u, true, false, WIDTH_DEFAULT, HEIGHT_DEFAULT, 2);
	int numPlanes = 400;	
	
	printf("Loading assets finished...\n");

	// init camera position and frame counter
	double xpos = 0;
	double ypos = 0;
	double fps_timer = 0;
	int fps_counter = 0;
	unsigned long long frameCounter = 0;
	float lastFrame = (float)glfwGetTime();

	// AntTweakBar init
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(WIDTH_DEFAULT, HEIGHT_DEFAULT);
	// Tw Types and definitions
	TwType TW_TYPE_VECTOR3F = TwDefineStruct("Vector3f", Vector3fMembers, 3, sizeof(glm::vec3), NULL, NULL);
	TwEnumVal Meshes[] = { {HEAD, "Head"}, {Option2, "Option2"} };
	TwType MeshTwType = TwDefineEnum("MeshType", Meshes, 2);

	bar = TwNewBar("Settings");
	TwDefine(" Settings help='This bar can be used to adapt some settings.' refresh=0.5 ");

	TwAddVarRW(bar, "Mesh", MeshTwType, &m_currentMesh, NULL);
	TwAddVarRW(bar, "Slice", TW_TYPE_INT32, &numPlanes, " label='# slices'  ");
	TwAddSeparator(bar, "", NULL);

	//TwAddButton(bar, "Camera", NULL, NULL, "");
	glm::vec3 camPos = camera.getPosition();
	glm::vec3 camDir = camera.getFront();
	TwAddVarRO(bar, "pos", TW_TYPE_VECTOR3F, &camPos, " label='Position' group='Camera'");
	TwAddVarRO(bar, "dir", TW_TYPE_VECTOR3F, &camDir, " label='Direction' group='Camera'");
	TwAddButton(bar, "Reset", ResetCamera, &camera, " group='Camera' ");
	TwDefine(" Settings/Camera opened=false ");
	TwAddSeparator(bar, "", NULL);
	
	//TwAddButton(bar, "Transfer function", NULL, NULL, "");
	TwAddVarRW(bar, "tf1color", TW_TYPE_COLOR3F, &color1, " label='TF1' group='TF'");
	TwAddVarRW(bar, "tf1pos", TW_TYPE_FLOAT, &pos[0], " label='TF1 position' group='TF' min=0 max=1 step=0.1");
	TwAddVarRW(bar, "tf2color", TW_TYPE_COLOR3F, &color2, " label='TF2' group='TF'");
	TwAddVarRW(bar, "tf2pos", TW_TYPE_FLOAT, &pos[1], " label='TF2 position' group='TF' min=0 max=1 step=0.1");
	TwAddVarRW(bar, "tf3color", TW_TYPE_COLOR3F, &color3, " label='TF3' group='TF'");
	TwAddVarRW(bar, "tf3pos", TW_TYPE_FLOAT, &pos[2], " label='TF3 position' group='TF' min=0 max=1 step=0.1");
	TwAddVarRW(bar, "tf4color", TW_TYPE_COLOR3F, &color4, " label='TF4' group='TF'");
	TwAddVarRW(bar, "tf4pos", TW_TYPE_FLOAT, &pos[3], " label='TF4 position' group='TF' min=0 max=1 step=0.1");
	   
	// render loop
	while (!glfwWindowShouldClose(window)) {

		// per-frame time
		float currentFrame = (float)glfwGetTime();
		frameDeltaTime = currentFrame - lastFrame;

		if (frameDeltaTime < 1.0 / fps)
			continue;

		fps_timer += frameDeltaTime;
		++fps_counter;
		if (fps_timer > 1.0f) {
			std::cout << frameCounter % numPlanes << "    " << float(fps_counter) / fps_timer << "     camerapos: " << glm::to_string(camera.getPosition()) <<  std::endl;
			
			fps_timer = 0;
			fps_counter = 0;
		}
		frameCounter++;
		lastFrame = currentFrame;		
				
		if (frameCounter % numPlanes == 0)
		{
			std::cout << "new frame" << "\n";
		}

		// update camera
		glfwGetCursorPos(window, &xpos, &ypos);
		if (allowMouseMove)
			camera.processMouseInput(window, xpos, ypos, frameDeltaTime);
		camera.processKeyInput(window, frameDeltaTime);
		camera.frameUpdate(frameDeltaTime);
		const glm::mat4& camView = camera.getViewMatrix();

		// render clear
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	
		//glClearColor(bgColor[0], bgColor[1], bgColor[2], 0.0f);
		glViewport(0, 0, WIDTH_DEFAULT, HEIGHT_DEFAULT);				

		glEnablei(GL_BLEND, 6);
		glEnablei(GL_BLEND, 4);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ZERO, GL_ONE, GL_ZERO);

		// construct first "view plane"
		float sphereRadius = glm::length(glm::vec3(0.5, 0.0, 0.0));			
		glm::mat3 viewMatrix3(glm::mat3(camView)); // TODO: there is thrown a warning?
		glm::vec3 originVS(camView * glm::vec4(0, 0, 0, 1));
		glm::vec3 middleOfObjectOnPlaneVS = originVS + glm::vec3(0.0, 0.0, std::min(-originVS.z, sphereRadius));		
			 
		//float d = glm::dot(middleOfObjectOnPlaneVS, originVS)/glm::length(originVS);
		glm::vec3 middleOfPlaneVS = middleOfObjectOnPlaneVS;// d* glm::normalize(originVS);
		glm::vec3 middleOfPlaneVSOpp = middleOfPlaneVS - glm::vec3(0, 0, sphereRadius + (middleOfPlaneVS.z - originVS.z));
		float planeDistance = glm::length(middleOfPlaneVS - middleOfPlaneVSOpp) / (float)numPlanes;
		// compute shader to init buffers - each frame
		computeShader.use();
		computeShader.setUniform("lightVSPos", lightVSPos);
		computeShader.setUniform("lightColor", lightColor);
		computeShader.setUniform("middleOfPlaneVS", middleOfPlaneVS);
		computeShader.setUniform("viewMatrix", camView);
		computeShader.setUniform("viewPos", glm::vec4(camera.getPosition(), 1.0));
		computeShader.setUniform("sphereRadius", sphereRadius);
		computeShader.setUniform("planeDistance", planeDistance);
		ilFBO.bindAllColorTexturesAsImageUnits();
		glDispatchCompute((GLuint)WIDTH_DEFAULT, (GLuint)HEIGHT_DEFAULT, 2);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// now main algorithm		
		ilFBO.setActive(false);		
		mainVolumeRender.use();
		mainVolumeRender.setUniform("sphereRadius", sphereRadius);
		mainVolumeRender.setUniform("planeDistance", planeDistance);
		mainVolumeRender.setUniform("inverseViewMatrix", glm::inverse(camView));
		mainVolumeRender.setUniform("viewMatrix", camView);
		mainVolumeRender.setUniform("middleOfPlaneVS", middleOfPlaneVS);
		std::vector<std::string> bufferNames{ "vpb", "vdb", "lb", "ldb", "cb", "mb", "debug" };
		int glLayer = 0;
		for (int i = 0; i < 6; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			mainVolumeRender.setUniform(bufferNames[i], i);
			glBindTexture(GL_TEXTURE_2D_ARRAY, ilFBO.getColorTexture(i));
			glObjectLabel(GL_TEXTURE, ilFBO.getColorTexture(i), -1, bufferNames[i].c_str());
		}
		int nextIndex = bufferNames.size();
		glActiveTexture(GL_TEXTURE0 + nextIndex);
		mainVolumeRender.setUniform("volTexture", nextIndex);
		glBindTexture(GL_TEXTURE_3D, volumeTexture);
		glObjectLabel(GL_TEXTURE, volumeTexture, -1, "volTexture");
		glActiveTexture(GL_TEXTURE0 + nextIndex + 1);
		mainVolumeRender.setUniform("colorTransfer", nextIndex + 1);
		glBindTexture(GL_TEXTURE_2D, colorTransfer);
		glObjectLabel(GL_TEXTURE, colorTransfer, -1, "colorTransfer");

		// %: debugging plane by plane draw
		for (int i = 0; i < frameCounter % numPlanes; i++)
		{
			glLayer = i % 2;
			mainVolumeRender.setUniform("glLayer", glLayer);
			float currentZ = middleOfPlaneVS.z - planeDistance * i;	
			mainVolumeRender.setUniform("currentZVS", currentZ);
				
			glClear(GL_DEPTH_BUFFER_BIT);
			glTextureBarrier();
			renderQuad();
		}		

		/// draw some buffer "suspended" where the initial plane would be
		debugDrawShader2.use();
		debugDrawShader2.setUniform("middleOfPlaneVS", middleOfPlaneVS);
		debugDrawShader2.setUniform("proj", camera.getProjection());
		debugDrawShader2.setUniform("glLayer", glLayer);
		debugDrawShader2.setUniform("sphereRadius", sphereRadius);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0 + 0);			
		glActiveTexture(GL_TEXTURE1);
		debugDrawShader2.setUniform("depthMap2", 1);
		glBindTexture(GL_TEXTURE_2D_ARRAY, ilFBO.getColorTexture(6));
		glObjectLabel(GL_TEXTURE, ilFBO.getColorTexture(6), -1, "output");
		//debugDrawShader2.setUniform("tex3", 1);
		//glBindTexture(GL_TEXTURE_2D, colorTransfer.id);		
		renderQuad();

		camPos = camera.getPosition();
		camDir = camera.getFront();
		TwSetParam(bar, "pos", "label", TW_PARAM_CSTRING, 1, "Position");
		TwSetParam(bar, "dir", "label", TW_PARAM_CSTRING, 1, "Direction");
		ChangeVolume(m_currentMesh);
		ChangeTransfer(color1, color2, color3, color4, pos);

		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		TwDraw(); //TODO: reorganize blending
		glDisable(GL_BLEND);		

		glfwSwapBuffers(window);
		glfwPollEvents();		
	}

    std::cout << "Hello World! (..and Goodbye ;) )\n"; 

	glfwTerminate();
	return 0;
}

// just used for info
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	std::cout << "framebuffer size " << width << "*" << height << "\n";
}

void processKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// ESCAPE - close window
	// TAB - lock/unlock mouse movement
	// F11 - full screen // TODO: should we include this?
	// Space - debug information of camera
	// WASD - camera movement forward, left, backwards, right

	TwEventKeyGLFW(key, action);

	if ((key == GLFW_KEY_1 || key == GLFW_KEY_2 || key == GLFW_KEY_3 || key == GLFW_KEY_4 || 
		key == GLFW_KEY_5 || key == GLFW_KEY_6 || key == GLFW_KEY_7 || key == GLFW_KEY_8 ||
		key == GLFW_KEY_9 || key == GLFW_KEY_0) && action == GLFW_PRESS)
	{
		TwEventCharGLFW(key, action);
	}

	if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
		allowMouseMove = !allowMouseMove;
		if (allowMouseMove)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //no cursor
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}	

	if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
		if (isFullScreen) {
			glfwSetWindowMonitor(window, NULL, 0, 0, WIDTH_DEFAULT, HEIGHT_DEFAULT, fps);
			isFullScreen = false;
		}
		else {
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, WIDTH_DEFAULT, HEIGHT_DEFAULT, fps);
			isFullScreen = true;
		}
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {

		std::cout << "Position: " << cameraPointer->getPosition().x << "," << cameraPointer->getPosition().y << "," << cameraPointer->getPosition().z << std::endl;
		std::cout << "Direction: " << cameraPointer->getFront().x << "," << cameraPointer->getFront().y << "," << cameraPointer->getFront().z << std::endl;
	}
}

void TW_CALL ResetCamera(void *clientData) {
	Camera *camera = static_cast<Camera *>(clientData); // camera pointer is stored in clientData
	camera->setPosition(glm::vec3(0.0f, 0.0f, 2.0f));
	camera->setFront(glm::vec3(0.0f, 0.0f, 1.0f));
}

int ChangeVolume(MESH_TYPE currentMesh)
{
	if (currentMesh == HEAD) {
		std::cout << "Choloepis_hoffmani" << std::endl;
		volumeTexture = volume1;
	}
	else if (currentMesh == Option2) {
		std::cout << "Option2" << std::endl;
		volumeTexture = volume1;
	}
	return volumeTexture;
}


void ChangeTransfer(glm::vec3 newColor1, glm::vec3 newColor2, glm::vec3 newColor3, glm::vec3 newColor4, glm::vec4 newPosition)
{
	if (newColor1 != oldColor1 || newColor2 != oldColor2 || newColor3 != oldColor3 || newColor4 != oldColor4 || newPosition != oldPos) {
		std::cout << "Change the colors" << std::endl;
		std::cout << "Old color" << oldColor1.x << "," << oldColor1.y << "," << oldColor1.z <<
			" New color" << newColor1.x << newColor1.y << newColor1.z << std::endl;
		std::cout << "Old color" << oldColor2.x << "," << oldColor2.y << "," << oldColor2.z <<
			" New color" << newColor2.x << "," << newColor2.y << "," << newColor2.z << std::endl;
		std::cout << "Old color" << oldColor3.x << "," << oldColor3.y << "," << oldColor3.z <<
			" New color" << newColor3.x << "," << newColor3.y << "," << newColor3.z << std::endl;
		std::cout << "Old color" << oldColor4.x << "," << oldColor4.y << "," << oldColor4.z <<
			" New color" << newColor4.x << "," << newColor4.y << "," << newColor4.z << std::endl;
		std::cout << "Old position" << oldPos.x << "," << oldPos.y << "," << oldPos.z <<
			" New position" << newPosition.x << "," << newPosition.y << "," << newPosition.z << std::endl;
		transferFunct.setColorsPos(newColor1, newColor2, newColor3, newColor4, newPosition);
		oldColor1 = newColor1;
		oldColor2 = newColor2;
		oldColor3 = newColor3;
		oldColor4 = newColor4;
		oldPos = newPosition;
		colorTransfer = transferFunct.getColorAlphaTransferTexture();
	}
}

void processMouseInput(GLFWwindow* window, double xpos, double ypos) {
	TwEventMousePosGLFW(xpos, ypos);
}

void processMouseButton(GLFWwindow * window, int button, int action, int mods)
{
	TwEventMouseButtonGLFW(button, action);
}

void processScrollInput(GLFWwindow* window, double xoffset, double yoffset) 
{	
	TwEventMouseWheelGLFW(yoffset);

	// TODO: is this used? it changes nothing
	/*if (fov >= 1.0f && fov <= 45.0f)
		fov -= (float)yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 100.0f)
		fov = 100.0f;*/
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad() {
	if (quadVAO == 0) {
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

static void APIENTRY DebugCallbackDefault(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam) {
	if (id == 131185 || id == 131218 || id == 131222) return; // ignore performance warnings (buffer uses GPU memory, shader recompilation) from nvidia
	std::string error = FormatDebugOutput(source, type, id, severity, message);
	std::cout << error << std::endl;
}

static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg) {
	std::stringstream stringStream;
	std::string sourceString;
	std::string typeString;
	std::string severityString;

	switch (source) {
	case GL_DEBUG_SOURCE_API: {
		sourceString = "API";
		break;
	}
	case GL_DEBUG_SOURCE_APPLICATION: {
		sourceString = "Application";
		break;
	}
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
		sourceString = "Window System";
		break;
	}
	case GL_DEBUG_SOURCE_SHADER_COMPILER: {
		sourceString = "Shader Compiler";
		break;
	}
	case GL_DEBUG_SOURCE_THIRD_PARTY: {
		sourceString = "Third Party";
		break;
	}
	case GL_DEBUG_SOURCE_OTHER: {
		sourceString = "Other";
		break;
	}
	default: {
		sourceString = "Unknown";
		break;
	}
	}
	switch (type) {
	case GL_DEBUG_TYPE_ERROR: {
		typeString = "Error";
		break;
	}
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
		typeString = "Deprecated Behavior";
		break;
	}
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
		typeString = "Undefined Behavior";
		break;
	}
	case GL_DEBUG_TYPE_PORTABILITY_ARB: {
		typeString = "Portability";
		break;
	}
	case GL_DEBUG_TYPE_PERFORMANCE: {
		typeString = "Performance";
		break;
	}
	case GL_DEBUG_TYPE_OTHER: {
		typeString = "Other";
		break;
	}
	default: {
		typeString = "Unknown";
		break;
	}
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: {
		severityString = "High";
		break;
	}
	case GL_DEBUG_SEVERITY_MEDIUM: {
		severityString = "Medium";
		break;
	}
	case GL_DEBUG_SEVERITY_LOW: {
		severityString = "Low";
		break;
	}
	default: {
		severityString = "Unknown";
		break;
	}
	}

	stringStream << "OpenGL Error: " << msg;
	stringStream << " [Source = " << sourceString;
	stringStream << ", Type = " << typeString;
	stringStream << ", Severity = " << severityString;
	stringStream << ", ID = " << id << "]";

	return stringStream.str();
}

GLFWwindow* setupAndConfigWindow()
{
	// init GLFW and return failed
	if (!glfwInit()) {
		printf("failed to initialize GLFW.\n");
		return nullptr;
	}

	// OpenGL settings
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, FBO::MULTI_SAMPLE_COUNT);
	glfwWindowHint(GLFW_REFRESH_RATE, fps);
#if _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	// create window
	window = glfwCreateWindow(WIDTH_DEFAULT, HEIGHT_DEFAULT, "VISII20", nullptr, nullptr);
	if (!window) {
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// initialize GLEW
	glewExperimental = true;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
//		EXIT_WITH_ERROR("Failed to init GLEW")
		return nullptr;
	}
	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	// enable OpenGL parameters
	glViewport(0, 0, WIDTH_DEFAULT, HEIGHT_DEFAULT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);  //MSAA
	glEnablei(GL_BLEND, 6);
	glEnablei(GL_BLEND, 4);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ZERO, GL_ONE, GL_ZERO);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_3D);

	// set window parameters 
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, processMouseInput);
	glfwSetScrollCallback(window, processScrollInput);
	glfwSetMouseButtonCallback(window, processMouseButton);
	glfwSetKeyCallback(window, processKeyboardInput);
	//glfwSetCharCallback(window, (GLFWcharfun)TwEventCharGLFW);
	if (!isFullScreen) {
		glfwSetWindowMonitor(window, NULL, 150, 150, WIDTH_DEFAULT, HEIGHT_DEFAULT, fps);
	}
	else {
		glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 150, 150, WIDTH_DEFAULT, HEIGHT_DEFAULT, fps);
	}

#if _DEBUG
	//Register Callback function
	glDebugMessageCallback(DebugCallbackDefault, NULL);
	//Synchronuous Callback - immediatly after error
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

	// reset color/ texture bindings
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return window;
}
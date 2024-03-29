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


unsigned int WIDTH_DEFAULT = 870;
unsigned int HEIGHT_DEFAULT = 600;

unsigned int modelWidth = 512;
unsigned int modelHeight = 512;

#define EXIT_WITH_ERROR(err) \
	std::cout << "ERROR: " << err << std::endl; \
	system("PAUSE"); \
	return EXIT_FAILURE;

// set parameters
float fov = 85.0f;
int fps = 20;
float frameDeltaTime = 0;
bool isFullScreen = false;
bool wireframe = false;

bool useIi = true;
bool useEnvMap = true;
bool useVRayRefraction = true;
bool useSpecularLight = true;
float shininess = 16.0;
bool autoRotate = false;
glm::vec3 lightVSPos;
glm::vec3 lightVSPosRef;
float speedLight = 0.5f;
glm::vec3 lightColor;
glm::vec3 refractionPos;
glm::vec4 refractionValue;

// create objects
GLFWwindow* window;
Camera* cameraPointer;
float bgColor[] = { 0.0f, 0.0f, 0.0f };
typedef enum {SLOT, CROC, LAMA, TURTLE, PYTHON} MESH_TYPE;
MESH_TYPE m_currentMesh = SLOT;
MESH_TYPE m_oldMesh = SLOT;
unsigned int volumeTexture;

unsigned int volumeTransfer;
unsigned int mediumTransfer;
TransferTableBuilder volumeTransferFunct{ TransferType::COLOR };
TransferTableBuilder mediumTransferFunct{ TransferType::MEDIUM };

glm::vec2 planeSides(0.6, 0.6);
int numPlanes = 400;
float voxelDepth = 1.0;

// method init
void renderQuad();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processMouseInput(GLFWwindow* window, double xpos, double ypos);
void processScrollInput(GLFWwindow* window, double xoffset, double yoffset);
void processMouseButton(GLFWwindow* window, int button, int action, int mods);
void processKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods);
void processKeyInputForLight(GLFWwindow* window, float frameDeltaTime);
void ChangeVolume();
void AddControlPoints(TwBar *twbar, glm::vec3 *pointNumbers, std::vector<double> *colors, std::vector<double> *positions,
	std::string name, std::string colorNameShort, std::string colorName, int number, int id);
void RemoveControlPoints(TwBar *twbar, glm::vec3 *pointNumbers, std::vector<double> *colors, std::vector<double> *positions,
	std::string name, std::string colorNameShort, std::string colorName, int number, int id);
void TW_CALL ResetLight(void *clientData);
void TW_CALL ResetCamera(void *clientData);
void TW_CALL ResetRefraction(void *clientData);
void TW_CALL ApplyTransferChanges(void *clientData);
void TW_CALL ResetTransferFunction(void *cliendData);
void setupAndConfigUI();
void setupVolumeTFUI();
void setupMediumTFUI();
void setupRefractionTFUI();
void checkUIValues();
GLFWwindow* setupAndConfigWindow();
static void APIENTRY DebugCallbackDefault(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);

TwBar *MeshBar;
TwBar *VolumeBar;
TwBar *MediumBar;
TwBar *RefractionBar;
TwStructMember Vector3fMembersV[] = {
	{ "x", TW_TYPE_FLOAT, offsetof(glm::vec3, x), "" },
	{ "y", TW_TYPE_FLOAT, offsetof(glm::vec3, y), "" },
	{ "z", TW_TYPE_FLOAT, offsetof(glm::vec3, z), "" }
};
TwStructMember Vector4fMembers[] = {
	{ "0", TW_TYPE_FLOAT, offsetof(glm::vec4, x), "" },
	{ "1", TW_TYPE_FLOAT, offsetof(glm::vec4, y), "" },
	{ "2", TW_TYPE_FLOAT, offsetof(glm::vec4, z), "" },
	{ "3", TW_TYPE_FLOAT, offsetof(glm::vec4, w), "" }
};
TwStructMember Vector3fMembers[] = {
	{ "0", TW_TYPE_FLOAT, offsetof(glm::vec3, x), "" },
	{ "1", TW_TYPE_FLOAT, offsetof(glm::vec3, y), "" },
	{ "2", TW_TYPE_FLOAT, offsetof(glm::vec3, z), "" }
};
TwStructMember TF5F[] = {
	{ "0", TW_TYPE_DOUBLE, sizeof(double) * 0, "" },
	{ "1", TW_TYPE_DOUBLE, sizeof(double) * 1, "" },
	{ "2", TW_TYPE_DOUBLE, sizeof(double) * 2, "" },
	{ "3", TW_TYPE_DOUBLE, sizeof(double) * 3, "" },
	{ "4", TW_TYPE_DOUBLE, sizeof(double) * 4, "" },
};
TwStructMember TF4F[] = {
	{ "0", TW_TYPE_DOUBLE, sizeof(double) * 0, "" },
	{ "1", TW_TYPE_DOUBLE, sizeof(double) * 1, "" },
	{ "2", TW_TYPE_DOUBLE, sizeof(double) * 2, "" },
	{ "3", TW_TYPE_DOUBLE, sizeof(double) * 3, "" },
};
TwStructMember TF3F[] = {
	{ "0", TW_TYPE_DOUBLE, sizeof(double) * 0, "" },
	{ "1", TW_TYPE_DOUBLE, sizeof(double) * 1, "" },
	{ "2", TW_TYPE_DOUBLE, sizeof(double) * 2, "" },
};
TwEnumVal Meshes[] = { {SLOT, "Two-toed Sloth"}, {CROC, "Cuban Crocodile"}, {LAMA, "Lama"}, {PYTHON, "Black-Headed Python"}, {TURTLE, "Northern Snapping Turtle"} };
TwType MeshTwType;
TwType TW_TYPE_VECTOR3;
TwType TW_TYPE_VECTOR3F;
TwType TW_TYPE_VECTOR4F;
TwType TW_TYPE_TF5F;
TwType TW_TYPE_TF4F;
TwType TW_TYPE_TF3F;

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

	// init shaders
	Shader basicRender("basic_render.vert", "basic_render.frag");	
	Shader mainVolumeRender("vol_vs.txt", "volume.frag", "volume.geom");
	Shader planeDrawShader("vol_vs.txt", "debug_texturedraw_fs.txt", "debug_texturedraw.geom");
	Shader computeShader("volume.comp");
	Shader computeEnvMapShader("envmap.comp");

	// load volume textures
	printf("Loading assets...\n");

	Texture environment;
	environment.createEmptyCubeTexture(modelWidth, modelHeight, 4, GL_RGBA32F);
	
	float boxSize = 2.0;
	Cube virtualRoom{ boxSize, boxSize, boxSize };
	glm::mat4 virtualRoomModelM;

	glm::mat4 envLookAt0 = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	glm::mat4 envLookAt1 = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	glm::mat4 envLookAt2 = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 envLookAt3 = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	glm::mat4 envLookAt4 = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	glm::mat4 envLookAt5 = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	computeEnvMapShader.use();
	computeEnvMapShader.setUniform("fl0", envLookAt0);
	computeEnvMapShader.setUniform("fl1", envLookAt1);
	computeEnvMapShader.setUniform("fl2", envLookAt2);
	computeEnvMapShader.setUniform("fl3", envLookAt3);
	computeEnvMapShader.setUniform("fl4", envLookAt4);
	computeEnvMapShader.setUniform("fl5", envLookAt5);
		
	volumeTexture = (Texture()).load3DTexture("assets/images/volumeData/Choloepis_hoffmani/PNG/CHOL", 512, 512, 1, 1, 441, ".png", 4);;

	// create transfer functions	
	volumeTransferFunct.initResources();
	mediumTransferFunct.initResources();
	volumeTransfer = volumeTransferFunct.getTransferTexture();	
	mediumTransfer = mediumTransferFunct.getTransferTexture();
	refractionPos = glm::vec3(0.25, 0.5, 0.75);
	refractionValue = glm::vec4(1.0, 1.34, 1.45, 1.8);

	// light settings
	lightVSPos = glm::vec3(0, 0, 15);
	lightVSPosRef = glm::vec3(lightVSPos.x, lightVSPos.y, 0);
	lightColor = glm::vec3(1.0, 1.0, 1.0) * 3.0f;	
	
	// main FBO: 6 buffers and 1 debug buffer
	FBO ilFBO(7u, true, false, modelWidth, modelHeight, 2);	
	
	printf("Loading assets finished...\n");

	// init camera position and frame counter
	double xpos = 0;
	double ypos = 0;
	double fps_timer = 0;
	int fps_counter = 0;
	unsigned long long frameCounter = 0;
	float lastFrame = (float)glfwGetTime();

	setupAndConfigUI();

	// render loop
	while (!glfwWindowShouldClose(window)) {

		// per-frame time
		float currentFrame = (float)glfwGetTime();
		frameDeltaTime = currentFrame - lastFrame;

		if (frameDeltaTime < 1.0 / fps)
			continue;

		fps_timer += frameDeltaTime;
		++fps_counter;
		if (fps_timer > 20.0f) {
			std::cout << "FPS:" << float(fps_counter) / fps_timer << std::endl;
			

			fps_timer = 0;
			fps_counter = 0;
		}
		frameCounter++;
		lastFrame = currentFrame;				

		// update camera
		glfwGetCursorPos(window, &xpos, &ypos);
		if (autoRotate)
			camera.autoRotate(frameDeltaTime);
		else
			camera.processKeyInput(window, frameDeltaTime);
		camera.frameUpdate(frameDeltaTime);
		const glm::mat4& camView = camera.getViewMatrix();
		const glm::mat4& inverseCamView = glm::inverse(camView);

		processKeyInputForLight(window, frameDeltaTime);
		checkUIValues();

		// render clear
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		
		glViewport(0, 0, modelWidth, modelHeight);

		glEnablei(GL_BLEND, 6);		
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ZERO, GL_ONE, GL_ZERO);

		// construct first "view plane"
		float sphereRadius = 0.5;
		
		glm::mat3 viewMatrix3(glm::mat3(camView));
		glm::vec3 originVS(camView * glm::vec4(0, 0, 0, 1));
		glm::vec3 middleOfObjectOnPlaneVS = originVS + glm::vec3(0.0, 0.0, std::min(-originVS.z, sphereRadius));		
			 
		glm::vec3 middleOfPlaneVS = middleOfObjectOnPlaneVS;
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
		computeShader.setUniform("planeSides", planeSides);
		computeShader.setUniform("planeWidth", (float)modelWidth);
		computeShader.setUniform("planeHeight", (float)modelHeight);
		ilFBO.bindAllColorTexturesAsImageUnits();
		glBindImageTexture(6, environment.id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glDispatchCompute((GLuint)modelWidth, (GLuint)modelHeight, 2);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// now main algorithm		
		ilFBO.setActive(false);		
		mainVolumeRender.use();
		mainVolumeRender.setUniform("sphereRadius", sphereRadius);
		mainVolumeRender.setUniform("planeDistance", planeDistance);
		mainVolumeRender.setUniform("inverseViewMatrix", inverseCamView);
		mainVolumeRender.setUniform("viewMatrix", camView);
		mainVolumeRender.setUniform("middleOfPlaneVS", middleOfPlaneVS);
		mainVolumeRender.setUniform("useIi",useIi);
		mainVolumeRender.setUniform("shininess", shininess);
		mainVolumeRender.setUniform("useVRayRefraction", useVRayRefraction);
		mainVolumeRender.setUniform("useSpec", useSpecularLight);
		mainVolumeRender.setUniform("planeSides", planeSides);
		mainVolumeRender.setUniform("dims", glm::ivec2(modelWidth, modelHeight));
		mainVolumeRender.setUniform("refractionPos", refractionPos);
		mainVolumeRender.setUniform("refractionValue", refractionValue);
		mainVolumeRender.setUniform("voxelDepth", voxelDepth);
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
		mainVolumeRender.setUniform("volumeTransfer", nextIndex + 1);
		glBindTexture(GL_TEXTURE_2D, volumeTransfer);
		glObjectLabel(GL_TEXTURE, volumeTransfer, -1, "volumeTransfer");
		glActiveTexture(GL_TEXTURE0 + nextIndex + 2);
		mainVolumeRender.setUniform("mediumTransfer", nextIndex + 2);
		glBindTexture(GL_TEXTURE_2D, mediumTransfer);
		glObjectLabel(GL_TEXTURE, mediumTransfer, -1, "mediumTransfer");

		// add mod %: debugging plane by plane draw
		for (int i = 0; i < numPlanes; i++)
		{
			glLayer = i % 2;
			mainVolumeRender.setUniform("glLayer", glLayer);
			float currentZ = middleOfPlaneVS.z - planeDistance * i;	
			mainVolumeRender.setUniform("currentZVS", currentZ);
				
			glClear(GL_DEPTH_BUFFER_BIT);
			glTextureBarrier();
			renderQuad();
		}
						
		if (useEnvMap) {
			computeEnvMapShader.use();
			computeEnvMapShader.setUniform("middleOfPlaneVS", middleOfPlaneVS);
			computeEnvMapShader.setUniform("middleOfPlaneVSOpp", middleOfPlaneVSOpp);

			computeEnvMapShader.setUniform("viewMatrix", camView);
			computeEnvMapShader.setUniform("inverseViewMatrix", inverseCamView);

			computeEnvMapShader.setUniform("planeWidth", (float)modelWidth);
			computeEnvMapShader.setUniform("planeHeight", (float)modelHeight);
			computeEnvMapShader.setUniform("planeSides", planeSides);
			computeEnvMapShader.setUniform("boxSize", boxSize);
			ilFBO.bindColorTextureAsImageUnit(2, 0, GL_READ_ONLY);
			ilFBO.bindColorTextureAsImageUnit(3, 1, GL_READ_ONLY);
			glBindImageTexture(2, environment.id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
			glDispatchCompute((GLuint)modelWidth, (GLuint)modelHeight, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			glTextureBarrier();
			glGenerateTextureMipmap(environment.id);
		}

		glViewport(0, 0, WIDTH_DEFAULT, HEIGHT_DEFAULT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		if (useEnvMap)
		{
			basicRender.use();
			basicRender.setUniform("lightColor", lightColor);
			basicRender.setUniform("modelMatrix", virtualRoomModelM);
			basicRender.setUniform("viewProjMatrix", camera.getProjection() * camView);

			glActiveTexture(GL_TEXTURE0 + 0);
			basicRender.setUniform("tex", 0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, environment.id);
			glCullFace(GL_FRONT);
			virtualRoom.draw();
			glCullFace(GL_BACK);
		}
		
		/// draw some buffer "suspended" where the initial plane would be
		planeDrawShader.use();
		planeDrawShader.setUniform("middleOfPlaneVS", middleOfPlaneVS);
		planeDrawShader.setUniform("planeSides", planeSides);
		planeDrawShader.setUniform("proj", camera.getProjection());
		planeDrawShader.setUniform("glLayer", glLayer);
		planeDrawShader.setUniform("sphereRadius", sphereRadius);		
		glActiveTexture(GL_TEXTURE0 + 0);			
		glActiveTexture(GL_TEXTURE1);
		planeDrawShader.setUniform("volumeResults", 1);
		glBindTexture(GL_TEXTURE_2D_ARRAY, ilFBO.getColorTexture(6));
		glObjectLabel(GL_TEXTURE, ilFBO.getColorTexture(6), -1, "output");

		renderQuad();

		ChangeVolume();
		
		TwDraw(); 
		glDisable(GL_BLEND);		

		glfwSwapBuffers(window);
		glfwPollEvents();		
	}

	glfwTerminate();
	return 0;
}

// just used for info
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	std::cout << "framebuffer size " << width << "*" << height << "\n";
}

void processKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// ESCAPE - close window
	// TAB - lock/unlock mouse movement // print control points
	// Space - debug information of camera
	// WASD - camera movement forward, left, backwards, right
	

	TwEventKeyGLFW(key, action);

	if ((key == GLFW_KEY_1 || key == GLFW_KEY_2 || key == GLFW_KEY_3 || key == GLFW_KEY_4 || 
		key == GLFW_KEY_5 || key == GLFW_KEY_6 || key == GLFW_KEY_7 || key == GLFW_KEY_8 ||
		key == GLFW_KEY_9 || key == GLFW_KEY_0 || key == GLFW_KEY_PERIOD) && action == GLFW_PRESS)
	{
		TwEventCharGLFW(key, action);
	}
		
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}	

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		useIi = !useIi;

		std::cout << "Position: " << cameraPointer->getPosition().x << "," << cameraPointer->getPosition().y << "," << cameraPointer->getPosition().z << std::endl;
		std::cout << "Direction: " << cameraPointer->getFront().x << "," << cameraPointer->getFront().y << "," << cameraPointer->getFront().z << std::endl;
		std::cout << "yaw: " << cameraPointer->getYaw() << " pitch " << cameraPointer->getPitch() << std::endl;
	}
}

void processKeyInputForLight(GLFWwindow* window, float frameDeltaTime) {
	
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		lightVSPos.x = std::min(planeSides.x, lightVSPos.x + speedLight * frameDeltaTime);
		std::cout << "Light pos" << glm::to_string(lightVSPos) << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		lightVSPos.x = std::max(-planeSides.x, lightVSPos.x - speedLight * frameDeltaTime);
		std::cout << "Light pos" << glm::to_string(lightVSPos) << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		lightVSPos.y = std::min(planeSides.y, lightVSPos.y + speedLight * frameDeltaTime);
		std::cout << "Light pos" << glm::to_string(lightVSPos) << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		lightVSPos.y = std::max(-planeSides.y, lightVSPos.y - speedLight * frameDeltaTime);
		std::cout << "Light pos" << glm::to_string(lightVSPos) << std::endl;
	}
	lightVSPosRef = glm::vec3(0) - glm::vec3(lightVSPos.x, lightVSPos.y, 0);
}

void checkUIValues() {
	if (lightColor.x < 0)
		lightColor.x = 0;
	if (lightColor.y < 0)
		lightColor.y = 0;
	if (lightColor.z < 0)
		lightColor.z = 0;

	if (refractionPos.x < 0)
		refractionPos.x = 0;
	if (refractionPos.x > 1)
		refractionPos.x = 1;
	if (refractionPos.y < refractionPos.x)
		refractionPos.y = refractionPos.x;
	if (refractionPos.y > 1)
		refractionPos.y = 1;
	if (refractionPos.z < refractionPos.y)
		refractionPos.z = refractionPos.y;
	if (refractionPos.z > 1)
		refractionPos.z = 1;

	if (refractionValue.x < 1)
		refractionValue.x = 1;
	if (refractionValue.x > 2.4)
		refractionValue.x = 2.4;
	if (refractionValue.y < refractionValue.x)
		refractionValue.y = refractionValue.x;
	if (refractionValue.y > 2.4)
		refractionValue.y = 2.4;
	if (refractionValue.z < refractionValue.y)
		refractionValue.z = refractionValue.y;
	if (refractionValue.z > 2.4)
		refractionValue.z = 2.4;
	if (refractionValue.w < refractionValue.z)
		refractionValue.w = refractionValue.z;
	if (refractionValue.w > 2.4)
		refractionValue.w = 2.4;
}

void TW_CALL ResetCamera(void *clientData) {
	cameraPointer->resetCamera();
}

void TW_CALL ResetLight(void *clientData) {
	lightVSPos = glm::vec3(0, 0, 5);
	lightVSPosRef = glm::vec3(0) - glm::vec3(lightVSPos.x, lightVSPos.y, 0);
	lightColor = glm::vec3(1.0, 1.0, 1.0) * 3.0f;
	useIi = true;
	useEnvMap = true;
	useVRayRefraction = true;
	useSpecularLight = true;
	shininess = 16.0;
	speedLight = 0.1f;
}

void TW_CALL ResetRefraction(void *clientData) {
	refractionPos = glm::vec3(0.25, 0.5, 0.75);
	refractionValue = glm::vec4(1.0, 1.34, 1.45, 1.8);
}

void TW_CALL ResetTransferFunction(void *clientData) {
	TransferTableBuilder* builder = static_cast<TransferTableBuilder*>(clientData);
	std::cout << "Reset " << builder->getType() << " transfer function" << std::endl;
	if (builder->getType() == 0) {
		volumeTransfer = builder->resetColorAlphaTransferTexture();
		TwRemoveAllVars(VolumeBar);
		setupVolumeTFUI();
	}
	if (builder->getType() == 1) {
		mediumTransfer = builder->resetColorAlphaTransferTexture();
	}
}

void TW_CALL ApplyTransferChanges(void * clientData)
{
	TransferTableBuilder* builder = static_cast<TransferTableBuilder*>(clientData);
	std::cout << "Update " << builder->getType() << " transfer function" << std::endl;
	if (builder->getType() == 0) {
		volumeTransfer = builder->getColorAlphaTransferTexture();
	}
	if (builder->getType() == 1) {
		mediumTransfer = builder->getColorAlphaTransferTexture();
	}
}

void TW_CALL AddPoint(void *clientData) {
	glm::vec3 *point = static_cast<glm::vec3 *>(clientData);

	if (point->y > 4)
		return;
	
	if (point->x == 1 && point->z == 0) {
		AddControlPoints(VolumeBar, &volumeTransferFunct.pointNumbers[0], &volumeTransferFunct.rCol,
			&volumeTransferFunct.rPos, " VolumeTFBar", "R", "Red", point->y, point->z);
	}
	if (point->x == 1 && point->z == 1) {
		AddControlPoints(MediumBar, &mediumTransferFunct.pointNumbers[0], &mediumTransferFunct.rCol,
			&mediumTransferFunct.rPos, " MediumTFBar", "R", "Red", point->y, point->z);
	}
	if (point->x == 2 && point->z == 0) {
		AddControlPoints(VolumeBar, &volumeTransferFunct.pointNumbers[1], &volumeTransferFunct.gCol,
			&volumeTransferFunct.gPos, " VolumeTFBar", "G", "Green", point->y, point->z);
	}
	if (point->x == 2 && point->z == 1) {
		AddControlPoints(MediumBar, &mediumTransferFunct.pointNumbers[1], &mediumTransferFunct.gCol,
			&mediumTransferFunct.gPos, " MediumTFBar", "G", "Green", point->y, point->z);
	}
	if (point->x == 3 && point->z == 0) {
		AddControlPoints(VolumeBar, &volumeTransferFunct.pointNumbers[2], &volumeTransferFunct.bCol,
			&volumeTransferFunct.bPos, " VolumeTFBar", "B", "Blue", point->y, point->z);
	}
	if (point->x == 3 && point->z == 1) {
		AddControlPoints(MediumBar, &mediumTransferFunct.pointNumbers[2], &mediumTransferFunct.bCol,
			&mediumTransferFunct.bPos, " MediumTFBar", "B", "Blue", point->y, point->z);
	}
	if (point->x == 4 && point->z == 0) {
		AddControlPoints(VolumeBar, &volumeTransferFunct.pointNumbers[3], &volumeTransferFunct.aCol,
			&volumeTransferFunct.aPos, " VolumeTFBar", "A", "Alpha", point->y, point->z);
	}
	if (point->x == 4 && point->z == 1) {
		AddControlPoints(MediumBar, &mediumTransferFunct.pointNumbers[3], &mediumTransferFunct.aCol,
			&mediumTransferFunct.aPos, " MediumTFBar", "A", "Alpha", point->y, point->z);
	}
}

void AddControlPoints(TwBar *twbar, glm::vec3 *pointNumbers, std::vector<double> *colors, std::vector<double> *positions,
	std::string name, std::string colorNameShort, std::string colorName, int number, int id) {
	TwRemoveVar(twbar, ("Color" + colorNameShort).c_str());
	TwRemoveVar(twbar, ("Position" + colorNameShort).c_str());
	colors[0].push_back(255);
	positions->back() = 0.90f;
	positions[0].push_back(1.0f);
	if (number == 4) {
		TwAddVarRW(twbar, ("Color" + colorNameShort).c_str(), TW_TYPE_TF5F, &colors[0][0], (" label='Color' group='" + colorName + "' ").c_str());
		TwAddVarRW(twbar, ("Position" + colorNameShort).c_str(), TW_TYPE_TF5F, &positions[0][0], (" label='Position' group='" + colorName + "' ").c_str());
		pointNumbers->y = 5;
	}
	else if (number == 3) {
		TwAddVarRW(twbar, ("Color" + colorNameShort).c_str(), TW_TYPE_TF4F, &colors[0][0], (" label='Color' group='" + colorName + "' ").c_str());
		TwAddVarRW(twbar, ("Position" + colorNameShort).c_str(), TW_TYPE_TF4F, &positions[0][0], (" label='Color' group='" + colorName + "' ").c_str());
		pointNumbers->y = 4;
	}
	TwDefine((name + "/Color" + colorNameShort + " opened=true ").c_str());
	TwDefine((name + "/Position" + colorNameShort + " opened=true ").c_str());
}

void RemoveControlPoints(TwBar *twbar, glm::vec3 *pointNumbers, std::vector<double> *colors, std::vector<double> *positions,
	std::string name, std::string colorNameShort, std::string colorName, int number, int id) {
	TwRemoveVar(twbar, ("Color" + colorNameShort).c_str());
	TwRemoveVar(twbar, ("Position" + colorNameShort).c_str());
	colors[0].pop_back();
	positions[0].pop_back();
	if (number == 4) {
		TwAddVarRW(twbar, ("Color" + colorNameShort).c_str(), TW_TYPE_TF3F, &colors[0][0], (" label='Color' group='" + colorName + "' ").c_str());
		TwAddVarRW(twbar, ("Position" + colorNameShort).c_str(), TW_TYPE_TF3F, &positions[0][0], (" label='Position' group='" + colorName + "' ").c_str());
		pointNumbers->y = 3;
	}
	else if (number == 5) {
		TwAddVarRW(twbar, ("Color" + colorNameShort).c_str(), TW_TYPE_TF4F, &colors[0][0], (" label='Color' group='" + colorName + "' ").c_str());
		TwAddVarRW(twbar, ("Position" + colorNameShort).c_str(), TW_TYPE_TF4F, &positions[0][0], (" label='Position' group='" + colorName + "' ").c_str());
		pointNumbers->y = 4;
	}
	TwDefine((name + "/Color" + colorNameShort + " opened=true ").c_str());
	TwDefine((name + "/Position" + colorNameShort + " opened=true ").c_str());
}

void TW_CALL DeletePoint(void *clientData) {
	glm::vec3 *point = static_cast<glm::vec3 *>(clientData);

	if (point->y <=3)
		return;
	
	if (point->x == 1 && point->z == 0) {
		RemoveControlPoints(VolumeBar, &volumeTransferFunct.pointNumbers[0], &volumeTransferFunct.rCol,
			&volumeTransferFunct.rPos, " VolumeTFBar", "R", "Red", point->y, point->z);
	}
	if (point->x == 1 && point->z == 1) {
		RemoveControlPoints(MediumBar, &mediumTransferFunct.pointNumbers[0], &mediumTransferFunct.rCol,
			&mediumTransferFunct.rPos, " MediumTFBar", "R", "Red", point->y, point->z);
	}
	if (point->x == 2 && point->z == 0) {
		RemoveControlPoints(VolumeBar, &volumeTransferFunct.pointNumbers[1], &volumeTransferFunct.gCol,
			&volumeTransferFunct.gPos, " VolumeTFBar", "G", "Green", point->y, point->z);
	}
	if (point->x == 2 && point->z == 1) {
		RemoveControlPoints(MediumBar, &mediumTransferFunct.pointNumbers[1], &mediumTransferFunct.gCol,
			&mediumTransferFunct.gPos, " MediumTFBar", "G", "Green", point->y, point->z);
	}
	if (point->x == 3 && point->z == 0) {
		RemoveControlPoints(VolumeBar, &volumeTransferFunct.pointNumbers[2], &volumeTransferFunct.bCol,
			&volumeTransferFunct.bPos, " VolumeTFBar", "B", "Blue", point->y, point->z);
	}
	if (point->x == 3 && point->z == 1) {
		RemoveControlPoints(MediumBar, &mediumTransferFunct.pointNumbers[2], &mediumTransferFunct.bCol,
			&mediumTransferFunct.bPos, " MediumTFBar", "B", "Blue", point->y, point->z);
	}
	if (point->x == 4 && point->z == 0) {
		RemoveControlPoints(VolumeBar, &volumeTransferFunct.pointNumbers[3], &volumeTransferFunct.aCol,
			&volumeTransferFunct.aPos, " VolumeTFBar", "A", "Alpha", point->y, point->z);
	}
	if (point->x == 4 && point->z == 1) {
		RemoveControlPoints(MediumBar, &mediumTransferFunct.pointNumbers[3], &mediumTransferFunct.aCol,
			&mediumTransferFunct.aPos, " MediumTFBar", "A", "Alpha", point->y, point->z);
	}
}

void ChangeVolume()
{	
	if (m_currentMesh != m_oldMesh) {
		glDeleteTextures(1, &volumeTexture);
		if (m_currentMesh == SLOT) {
			std::cout << "Loading SLOT" << std::endl;
			volumeTexture = (Texture()).load3DTexture("assets/images/volumeData/Choloepis_hoffmani/PNG/CHOL", 512, 512, 1, 1, 441, ".png", 4);;
		}
		else if (m_currentMesh == CROC) {
			std::cout << "Loading Cuban Crocodile" << std::endl;
			volumeTexture = (Texture()).load3DTexture("assets/images/volumeData/Crocodylus_rhombifer/PNG2/Crhombifer_", 512, 512, 1, 1, 711, ".png", 4);;
		}
		else if (m_currentMesh == LAMA) {
			std::cout << "Loading Lama" << std::endl;
			volumeTexture = (Texture()).load3DTexture("assets/images/volumeData/Lama_glama/PNG2/lama", 512, 512, 1, 1, 426, ".png", 4);
		}
		else if (m_currentMesh == PYTHON) {
			std::cout << "Loading Python" << std::endl;
			volumeTexture = (Texture()).load3DTexture("assets/images/volumeData/Aspidites_melanocephalus/PNG2/aspid", 512, 512, 1, 1, 510, ".png", 4);
		}
		else if (m_currentMesh == TURTLE) {
			std::cout << "Loading Turtle" << std::endl;
			volumeTexture = (Texture()).load3DTexture("assets/images/volumeData/Elseya_dentata/PNG2/TURTL", 512, 512, 1, 1, 246, ".png", 4);
		}

		m_oldMesh = m_currentMesh;
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

	cameraPointer->processScroll(yoffset);

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
	//glEnable(GL_MULTISAMPLE);  //MSAA
	glEnablei(GL_BLEND, 6);	
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

void setupAndConfigUI()
{
	// AntTweakBar init
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(WIDTH_DEFAULT, HEIGHT_DEFAULT);
	// Tw Types and definitions
	TW_TYPE_VECTOR3 = TwDefineStruct("Vector3fV", Vector3fMembersV, 3, sizeof(glm::vec3), NULL, NULL);
	TW_TYPE_VECTOR3F = TwDefineStruct("Vector3f", Vector3fMembers, 3, sizeof(glm::vec3), NULL, NULL);
	TW_TYPE_VECTOR4F = TwDefineStruct("Vector4f", Vector4fMembers, 4, sizeof(glm::vec4), NULL, NULL);
	TW_TYPE_TF5F = TwDefineStruct("TF5F", TF5F, 5, sizeof(double) * 5, NULL, NULL);
	TW_TYPE_TF4F = TwDefineStruct("TF4F", TF4F, 4, sizeof(double) * 4, NULL, NULL);
	TW_TYPE_TF3F = TwDefineStruct("TF3F", TF3F, 3, sizeof(double) * 3, NULL, NULL);
	MeshTwType = TwDefineEnum("MeshType", Meshes, 5);

	MeshBar = TwNewBar("Settings");
	TwAddVarRW(MeshBar, "Meshes", MeshTwType, &m_currentMesh, " group='Mesh' label='Mesh Type' ");
	TwAddVarRW(MeshBar, "Slice", TW_TYPE_INT32, &numPlanes, " group='Mesh' label='# Slices'  ");	
	TwDefine(" Settings/Mesh opened=true ");
	TwAddSeparator(MeshBar, "", NULL);	
	TwAddVarRW(MeshBar, "Intensity Correction", TW_TYPE_BOOLCPP, &useIi, " label='Intensity Cor.' group='Light'");
	TwAddVarRW(MeshBar, "Render Environement", TW_TYPE_BOOLCPP, &useEnvMap, " label='Render Environment' group='Light'");
	TwAddVarRW(MeshBar, "View Ray Refraction", TW_TYPE_BOOLCPP, &useVRayRefraction, " label='View Ref.' group='Light'");
	TwAddVarRW(MeshBar, "Specular Light", TW_TYPE_BOOLCPP, &useSpecularLight, " label='Specular Comp.' group='Light'");
	TwAddVarRW(MeshBar, "Spec. Exp.", TW_TYPE_FLOAT, &shininess, " label='Spec. Exp.' group='Light' step=0.1");
	TwAddVarRW(MeshBar, "Light Speed", TW_TYPE_FLOAT, &speedLight, " group='Light' min=0 max=0.2 step=0.01");
	TwAddVarRW(MeshBar, "Light Color", TW_TYPE_VECTOR3, &lightColor, " group='Light' ");
	TwAddVarRO(MeshBar, "Light Pos", TW_TYPE_DIR3F, &lightVSPosRef, " group='Light' ");
	TwAddButton(MeshBar, "Reset Light", ResetLight, NULL, " group='Light' ");
	TwDefine(" Settings/Light opened=true ");
	TwAddSeparator(MeshBar, "", NULL);
	TwAddVarRW(MeshBar, "Camera Speed", TW_TYPE_FLOAT, &cameraPointer->speed, " min=0.1 max=20.0 step=0.1 group='Camera' ");
	TwAddVarRW(MeshBar, "Rotate", TW_TYPE_BOOL8, &autoRotate, " group='Camera' " );
	TwAddButton(MeshBar, "Reset Camera", ResetCamera, NULL, " group='Camera' ");
	TwDefine(" Settings/Camera opened=true ");
	TwAddSeparator(MeshBar, "", NULL);
	TwDefine(" Settings label='Settings' refresh=0.5 size='200 360' position='10 10' iconpos=bottomleft iconalign=vertical");

	VolumeBar = TwNewBar("VolumeTFBar");
	TwDefine(" VolumeTFBar label='Volume TF' refresh=0.5 size='200 280' valueswidth=100 position='660 10' iconpos=bottomleft iconalign=vertical");
	setupVolumeTFUI();

	MediumBar = TwNewBar("MediumTFBar");
	TwDefine(" MediumTFBar label='Medium TF' refresh=0.5 size='200 280' valueswidth=100 position='660 310' iconpos=bottomleft iconalign=vertical");
	setupMediumTFUI();

	RefractionBar = TwNewBar("RefBar");
	TwDefine(" RefBar label='Refraction TF' refresh=0.5 size='200 200' valueswidth=100 position='30 380' iconpos=bottomleft iconalign=vertical");
	setupRefractionTFUI();
	
}

void setupVolumeTFUI() {


	int colorSize = volumeTransferFunct.rCol.size();
	int posSize = volumeTransferFunct.rPos.size();
	TwAddButton(VolumeBar, "AddPointR", AddPoint, &volumeTransferFunct.pointNumbers[0], "group = 'Red' label='Add point'");
	TwAddButton(VolumeBar, "RemovePointR", DeletePoint, &volumeTransferFunct.pointNumbers[0], "group ='Red' label='Remove point'");
	TwAddVarRW(VolumeBar, "ColorR", colorSize == 3 ? TW_TYPE_TF3F : (colorSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F) , &volumeTransferFunct.rCol[0], "group='Red' label='Color'");
	TwAddVarRW(VolumeBar, "PositionR", posSize == 3 ? TW_TYPE_TF3F : (posSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &volumeTransferFunct.rPos[0], "group='Red' label='Position' ");


	TwDefine(" VolumeTFBar/Red opened=false ");
	TwDefine(" VolumeTFBar/ColorR opened=true ");
	TwDefine(" VolumeTFBar/PositionR opened=true ");

	colorSize = volumeTransferFunct.gCol.size();
	posSize = volumeTransferFunct.gPos.size();
	TwAddButton(VolumeBar, "AddPointG", AddPoint, &volumeTransferFunct.pointNumbers[1], "group = 'Green' label='Add point'");
	TwAddButton(VolumeBar, "RemovePointG", DeletePoint, &volumeTransferFunct.pointNumbers[1], "group ='Green' label='Remove point'");
	TwAddVarRW(VolumeBar, "ColorG", colorSize == 3 ? TW_TYPE_TF3F : (colorSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &volumeTransferFunct.gCol[0], "group='Green' label='Color' ");
	TwAddVarRW(VolumeBar, "PositionG", posSize == 3 ? TW_TYPE_TF3F : (posSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &volumeTransferFunct.gPos[0], "group='Green' label='Position' ");
	TwDefine(" VolumeTFBar/Green opened=false ");
	TwDefine(" VolumeTFBar/ColorG opened=true ");
	TwDefine(" VolumeTFBar/PositionG opened=true ");
	colorSize = volumeTransferFunct.bCol.size();
	posSize = volumeTransferFunct.bPos.size();
	TwAddButton(VolumeBar, "AddPointB", AddPoint, &volumeTransferFunct.pointNumbers[2], "group = 'Blue' label='Add point'");
	TwAddButton(VolumeBar, "RemovePointB", DeletePoint, &volumeTransferFunct.pointNumbers[2], "group ='Blue' label='Remove point'");
	TwAddVarRW(VolumeBar, "ColorB", colorSize == 3 ? TW_TYPE_TF3F : (colorSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &volumeTransferFunct.bCol[0], "group='Blue' label='Color' ");
	TwAddVarRW(VolumeBar, "PositionB", posSize == 3 ? TW_TYPE_TF3F : (posSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &volumeTransferFunct.bPos[0], "group='Blue' label='Position'");
	TwDefine(" VolumeTFBar/Blue opened=false ");
	TwDefine(" VolumeTFBar/ColorB opened=true ");
	TwDefine(" VolumeTFBar/PositionB opened=true ");
	colorSize = volumeTransferFunct.aCol.size();
	posSize = volumeTransferFunct.aPos.size();
	TwAddButton(VolumeBar, "AddPointA", AddPoint, &volumeTransferFunct.pointNumbers[3], "group = 'Alpha' label='Add point'");
	TwAddButton(VolumeBar, "RemovePointA", DeletePoint, &volumeTransferFunct.pointNumbers[3], "group ='Alpha' label='Remove point'");
	TwAddVarRW(VolumeBar, "ColorA", colorSize == 3 ? TW_TYPE_TF3F : (colorSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &volumeTransferFunct.aCol[0], "group='Alpha' label='Color' ");
	TwAddVarRW(VolumeBar, "PositionA", posSize == 3 ? TW_TYPE_TF3F : (posSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &volumeTransferFunct.aPos[0], "group='Alpha' label='Position'");
	TwDefine(" VolumeTFBar/Alpha opened=false");
	TwDefine(" VolumeTFBar/ColorA opened=true ");
	TwDefine(" VolumeTFBar/PositionA opened=true ");
	TwAddButton(VolumeBar, "Apply Volume TF", ApplyTransferChanges, &volumeTransferFunct, NULL);
	TwAddButton(VolumeBar, "Reset Volume TF", ResetTransferFunction, &volumeTransferFunct, NULL);
}

void setupMediumTFUI() {
	
	int colorSize = mediumTransferFunct.rCol.size();
	int posSize = mediumTransferFunct.rPos.size();
	TwAddButton(MediumBar, "AddPointR", AddPoint, &mediumTransferFunct.pointNumbers[0], "group = 'Red' label='Add point'");
	TwAddButton(MediumBar, "RemovePointR", DeletePoint, &mediumTransferFunct.pointNumbers[0], "group ='Red' label='Remove point'");
	TwAddVarRW(MediumBar, "ColorR", colorSize == 3 ? TW_TYPE_TF3F : (colorSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &mediumTransferFunct.rCol[0], "group='Red' label='Color'");
	TwAddVarRW(MediumBar, "PositionR", posSize == 3 ? TW_TYPE_TF3F : (posSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &mediumTransferFunct.rPos[0], "group='Red' label='Position' ");
	TwDefine(" MediumTFBar/Red opened=false ");
	TwDefine(" MediumTFBar/ColorR opened=true ");
	TwDefine(" MediumTFBar/PositionR opened=true ");
	colorSize = mediumTransferFunct.gCol.size();
	posSize = mediumTransferFunct.gPos.size();
	TwAddButton(MediumBar, "AddPointG", AddPoint, &mediumTransferFunct.pointNumbers[1], "group = 'Green' label='Add point'");
	TwAddButton(MediumBar, "RemovePointG", DeletePoint, &mediumTransferFunct.pointNumbers[1], "group ='Green' label='Remove point'");
	TwAddVarRW(MediumBar, "ColorG", colorSize == 3 ? TW_TYPE_TF3F : (colorSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &mediumTransferFunct.gCol[0], "group='Green' label='Color' ");
	TwAddVarRW(MediumBar, "PositionG", posSize == 3 ? TW_TYPE_TF3F : (posSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &mediumTransferFunct.gPos[0], "group='Green' label='Position' ");
	TwDefine(" MediumTFBar/Green opened=false ");
	TwDefine(" MediumTFBar/ColorG opened=true ");
	TwDefine(" MediumTFBar/PositionG opened=true ");
	colorSize = mediumTransferFunct.bCol.size();
	posSize = mediumTransferFunct.bPos.size();
	TwAddButton(MediumBar, "AddPointB", AddPoint, &mediumTransferFunct.pointNumbers[2], "group = 'Blue' label='Add point'");
	TwAddButton(MediumBar, "RemovePointB", DeletePoint, &mediumTransferFunct.pointNumbers[2], "group ='Blue' label='Remove point'");
	TwAddVarRW(MediumBar, "ColorB", colorSize == 3 ? TW_TYPE_TF3F : (colorSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &mediumTransferFunct.bCol[0], "group='Blue' label='Color' ");
	TwAddVarRW(MediumBar, "PositionB", posSize == 3 ? TW_TYPE_TF3F : (posSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &mediumTransferFunct.bPos[0], "group='Blue' label='Position'");
	TwDefine(" MediumTFBar/Blue opened=false ");
	TwDefine(" MediumTFBar/ColorB opened=true ");
	TwDefine(" MediumTFBar/PositionB opened=true ");
	colorSize = mediumTransferFunct.aCol.size();
	posSize = mediumTransferFunct.aPos.size();
	TwAddButton(MediumBar, "AddPointA", AddPoint, &mediumTransferFunct.pointNumbers[3], "group = 'Alpha' label='Add point'");
	TwAddButton(MediumBar, "RemovePointA", DeletePoint, &mediumTransferFunct.pointNumbers[3], "group ='Alpha' label='Remove point'");
	TwAddVarRW(MediumBar, "ColorA", colorSize == 3 ? TW_TYPE_TF3F : (colorSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &mediumTransferFunct.aCol[0], "group='Alpha' label='Color' ");
	TwAddVarRW(MediumBar, "PositionA", posSize == 3 ? TW_TYPE_TF3F : (posSize == 4 ? TW_TYPE_TF4F : TW_TYPE_TF5F), &mediumTransferFunct.aPos[0], "group='Alpha' label='Position'");
	TwDefine(" MediumTFBar/Alpha opened=false");
	TwDefine(" MediumTFBar/ColorA opened=true ");
	TwDefine(" MediumTFBar/PositionA opened=true ");
	TwAddButton(MediumBar, "Apply Medium TF", ApplyTransferChanges, &mediumTransferFunct, NULL);
	TwAddButton(MediumBar, "Reset Medium TF", ResetTransferFunction, &mediumTransferFunct, NULL);
}

void setupRefractionTFUI() {

	TwAddVarRW(RefractionBar, "ValuesRef", TW_TYPE_VECTOR4F, &refractionValue, " group='Refraction' label='Values' ");
	TwAddVarRW(RefractionBar, "ValuesPos", TW_TYPE_VECTOR3F, &refractionPos, " group='Refraction' label='Position' ");
	TwAddButton(RefractionBar, "Reset Refraction", ResetRefraction, NULL, " group='Refraction' label='Reset Refraction' ");
	TwDefine(" RefBar/Refraction opened=true ");
}
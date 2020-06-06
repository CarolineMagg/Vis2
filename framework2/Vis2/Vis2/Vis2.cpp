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
unsigned int HEIGHT_DEFAULT = 580;

unsigned int modelWidth = 512/4;
unsigned int modelHeight = 512/4;

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
bool useVRayRefraction = true;
bool useSpecularLight = true;
float shininess = 16.0;
bool autoRotate = false;


// create objects
GLFWwindow* window;
Camera* cameraPointer;
float bgColor[] = { 0.0f, 0.0f, 0.0f };
typedef enum {SLOT, CROC, LAMA, TURTLE, PYTHON} MESH_TYPE;
MESH_TYPE m_currentMesh = SLOT;
MESH_TYPE m_oldMesh = SLOT;
int volumeTexture;
int volume1;
int volume2;
int volume3;
int volume4;
int volume5;
unsigned int volumeTransfer;
unsigned int mediumTransfer;
glm::vec4 colors[] = {
		glm::vec4(0.0, 0.0, 0.0, 0.0), // color1 
		glm::vec4(0.47, 0.47, 0.47, 0.10), // color2
		glm::vec4(0.82, 0.82, 0.82, 0.4), // color3
		glm::vec4(1.0, 1.0, 1.0, 1.0), // color4
		glm::vec4(0.0, 0.3, 0.6, 1.0) // position
};
glm::vec4 colors2[] = {
		glm::vec4(1.0, 1.0, 1.0, 1.0), // color1 
		glm::vec4(0.98, 0.98, 0.98, 1.0), // color2
		glm::vec4(0.97, 0.97, 0.97, 1.0), // color3
		glm::vec4(0.95, 0.95, 0.95, 1.0), // color4
		glm::vec4(0.0, 0.3, 0.6, 1.0) // position
};
TransferTableBuilder volumeTransferFunct;
TransferTableBuilder mediumTransferFunct;


int numPlanes = 400;

// method init
void renderQuad();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processMouseInput(GLFWwindow* window, double xpos, double ypos);
void processScrollInput(GLFWwindow* window, double xoffset, double yoffset);
void processMouseButton(GLFWwindow* window, int button, int action, int mods);
void processKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods);
void ChangeVolume();
void TW_CALL ResetCamera(void *clientData);
void TW_CALL ApplyVolTransferChanges(void *clientData);
void TW_CALL ApplyMedTransferChanges2(void *clientData);
void TW_CALL ResetVolTransferFunction(void *cliendData);
void TW_CALL ResetMedTransferFunction(void *cliendData);
void TW_CALL LoadMeshes(void *clientData);
void setupAndConfigUI();
GLFWwindow* setupAndConfigWindow();
static void APIENTRY DebugCallbackDefault(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);

TwBar *MeshBar;
TwBar *VolumeBar;
TwBar *MediumBar;

TwStructMember Vector3fMembers[] = {
	{ "x", TW_TYPE_DOUBLE, offsetof(glm::vec3, x), "" },
	{ "y", TW_TYPE_DOUBLE, offsetof(glm::vec3, y), "" },
	{ "z", TW_TYPE_DOUBLE, offsetof(glm::vec3, z), "" }
}; 
TwStructMember Vector4fMembers[] = {
	{ "x", TW_TYPE_DOUBLE, offsetof(glm::vec4, x), "" },
	{ "y", TW_TYPE_DOUBLE, offsetof(glm::vec4, y), "" },
	{ "z", TW_TYPE_DOUBLE, offsetof(glm::vec4, z), "" },
	{ "a", TW_TYPE_DOUBLE, offsetof(glm::vec4, w), "" }
};
TwStructMember TF5F[] = {
	{ "0", TW_TYPE_DOUBLE, 0, "" },
	{ "1", TW_TYPE_DOUBLE, 4, "" },
	{ "2", TW_TYPE_DOUBLE, 8, "" },
	{ "3", TW_TYPE_DOUBLE, 12, "" },
	{ "4", TW_TYPE_DOUBLE, 16, "" },
};
TwStructMember TF4F[] = {
	{ "0", TW_TYPE_DOUBLE, 0, "" },
	{ "1", TW_TYPE_DOUBLE, 4, "" },
	{ "2", TW_TYPE_DOUBLE, 8, "" },
	{ "3", TW_TYPE_DOUBLE, 12, "" },
};
TwStructMember TF3F[] = {
	{ "0", TW_TYPE_DOUBLE, 0, "" },
	{ "1", TW_TYPE_DOUBLE, 4, "" },
	{ "2", TW_TYPE_DOUBLE, 8, "" },
};
TwEnumVal Meshes[] = { {SLOT, "Two-toed Sloth"}, {CROC, "American Alligator"}, {LAMA, "Lama"}, {PYTHON, "Black-Headed Python"}, {TURTLE, "Northern Snapping Turtle"} };
TwType MeshTwType;
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

	// load volume textures
	printf("Loading assets...\n");
	volume1 = (Texture()).load3DTexture("assets/images/volumeData/Choloepis_hoffmani/PNG/CHOL", 512, 512, 1, 1, 441, ".png", 4);
	volumeTexture = volume1;

	// create transfer functions
	volumeTransferFunct = TransferTableBuilder(1);
	volumeTransfer = volumeTransferFunct.getTransfer();
	mediumTransferFunct = TransferTableBuilder(2);
	mediumTransfer = mediumTransferFunct.getTransfer();

	// light settings
	glm::vec3 lightVSPos = glm::vec3(0, 0, 20);
	glm::vec3 lightColor = glm::vec3(1.0, 1.0, 1.0) * 3.0f;
	glm::vec3 voxelSize = glm::vec3(1.0f, 1.0f, 0.2409f / 0.1284f);
	
	// main FBO: 6 buffers and 1 debug buffer
	FBO ilFBO(7u, true, false, modelWidth, modelHeight, 2);
	int numPlanes = 400;	
	
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
		if (fps_timer > 1.0f) {
			std::cout << float(fps_counter) / fps_timer << " camerapos: " << glm::to_string(camera.getPosition()) << std::endl;
			std::cout << float(fps_counter) / fps_timer << " camerafront: " << glm::to_string(camera.getFront()) << std::endl;
			std::cout << "yaw: " << camera.getYaw() << " pitch " << camera.getPitch() << std::endl;

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


		// render clear
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);		
		glViewport(0, 0, modelWidth, modelHeight);

		glEnablei(GL_BLEND, 6);		
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
		computeShader.setUniform("planeWidth", (float)modelWidth);
		computeShader.setUniform("planeHeight", (float)modelHeight);
		ilFBO.bindAllColorTexturesAsImageUnits();
		glDispatchCompute((GLuint)modelWidth, (GLuint)modelHeight, 2);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// now main algorithm		
		ilFBO.setActive(false);		
		mainVolumeRender.use();
		mainVolumeRender.setUniform("sphereRadius", sphereRadius);
		mainVolumeRender.setUniform("planeDistance", planeDistance);
		mainVolumeRender.setUniform("inverseViewMatrix", glm::inverse(camView));
		mainVolumeRender.setUniform("viewMatrix", camView);
		mainVolumeRender.setUniform("middleOfPlaneVS", middleOfPlaneVS);
		mainVolumeRender.setUniform("useIi",useIi);
		mainVolumeRender.setUniform("shininess", shininess);
		mainVolumeRender.setUniform("useVRayRefraction", useVRayRefraction);
		mainVolumeRender.setUniform("useSpec", useSpecularLight);		
		mainVolumeRender.setUniform("dims", glm::ivec2(modelWidth, modelHeight));
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

		// %: debugging plane by plane draw
		for (int i = 0; i <   numPlanes; i++)
		{
			glLayer = i % 2;
			mainVolumeRender.setUniform("glLayer", glLayer);
			float currentZ = middleOfPlaneVS.z - planeDistance * i;	
			mainVolumeRender.setUniform("currentZVS", currentZ);
				
			glClear(GL_DEPTH_BUFFER_BIT);
			glTextureBarrier();
			renderQuad();
		}
		
		glViewport(0, 0, WIDTH_DEFAULT, HEIGHT_DEFAULT);

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
		/*debugDrawShader2.setUniform("tex3", 1);
		glBindTexture(GL_TEXTURE_3D, volumeTexture);	*/	
		renderQuad();

		ChangeVolume();
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
		useIi = !useIi;

		std::cout << "Position: " << cameraPointer->getPosition().x << "," << cameraPointer->getPosition().y << "," << cameraPointer->getPosition().z << std::endl;
		std::cout << "Direction: " << cameraPointer->getFront().x << "," << cameraPointer->getFront().y << "," << cameraPointer->getFront().z << std::endl;
	}
}

void TW_CALL ResetCamera(void *clientData) {
	cameraPointer->resetCamera();
}

void TW_CALL ResetVolTransferFunction(void *clientData) {
	std::cout << "Reset volume transfer function" << std::endl;
	TransferTableBuilder *tf = static_cast<TransferTableBuilder *>(clientData);
	volumeTransfer = tf->resetColorAlphaTransferTexture();
}

void TW_CALL ResetMedTransferFunction(void *clientData) {
	std::cout << "Reset medium transfer function" << std::endl;
	TransferTableBuilder *tf = static_cast<TransferTableBuilder *>(clientData);
	mediumTransfer = tf->resetColorAlphaTransferTexture();
}

void TW_CALL ApplyVolTransferChanges(void * clientData)
{
	std::cout << "Update volume transfer function" << std::endl;
	TransferTableBuilder *tf = static_cast<TransferTableBuilder *>(clientData);
	volumeTransfer = tf->getColorAlphaTransferTexture();
}

void TW_CALL ApplyMedTransferChanges2(void * clientData)
{
	std::cout << "Update medium transfer function" << std::endl;
	TransferTableBuilder *tf = static_cast<TransferTableBuilder *>(clientData);
	mediumTransfer = tf->getColorAlphaTransferTexture();
}

void TW_CALL LoadMeshes(void *clientData) 
{
	MeshTwType = TwDefineEnum("MeshType", Meshes, 3);
	volume2 = (Texture()).load3DTexture("assets/images/volumeData/Alligator_mississippiensis/PNG/allig", 1204, 512, 1, 1, 135, ".png", 4);
	volume3 = (Texture()).load3DTexture("assets/images/volumeData/Lama_glama/PNG/lama", 1204, 1204, 1, 1, 426, ".png", 4);
	/*volume4 = (Texture()).load3DTexture("assets/images/volumeData/Aspidites_melanocephalus/PNG/aspid", 512, 512, 1, 1, 510, ".png", 4);
	volume5 = (Texture()).load3DTexture("assets/images/volumeData/Elseya_dentata/PNG/TURTL", 512, 512, 1, 1, 246, ".png", 4);*/

}

void TW_CALL AddPoint(void *clientData) {
	glm::vec2 *point = static_cast<glm::vec2 *>(clientData);

	if (point->y > 4)
		return;

	if (point->x == 1) {
		volumeTransferFunct.rCol.push_back(255);
		volumeTransferFunct.rPos.push_back(1.0f);
		TwRemoveVar(VolumeBar, "ColorR");
		TwRemoveVar(VolumeBar, "PositionR");
		if (point->y == 4) {
			TwAddVarRW(VolumeBar, "ColorR", TW_TYPE_TF5F, &volumeTransferFunct.rCol, " label='color' group='R' ");
			TwAddVarRW(VolumeBar, "PositionR", TW_TYPE_TF5F, &volumeTransferFunct.rPos, "label='position' group='R' ");
			volumeTransferFunct.pointNumbers[0] = glm::vec2(1, 5);
		}
		else if (point->y == 3) {
			TwAddVarRW(VolumeBar, "ColorR", TW_TYPE_TF4F, &volumeTransferFunct.rCol, " label='color' group='R' ");
			TwAddVarRW(VolumeBar, "PositionR", TW_TYPE_TF4F, &volumeTransferFunct.rPos, " label='position' group='R' ");
			volumeTransferFunct.pointNumbers[0] = glm::vec2(1, 4);
		}
	}
	if (point->x == 2) {
		volumeTransferFunct.gCol.push_back(255);
		volumeTransferFunct.gPos.push_back(1.0f);
		TwRemoveVar(VolumeBar, "ColorG");
		TwRemoveVar(VolumeBar, "PositionG");
		if (point->y == 4) {
			TwAddVarRW(VolumeBar, "ColorG", TW_TYPE_TF5F, &volumeTransferFunct.rCol, " label='color' group='G' ");
			TwAddVarRW(VolumeBar, "PositionG", TW_TYPE_TF5F, &volumeTransferFunct.rPos, " label='position' group='G' ");
			volumeTransferFunct.pointNumbers[1] = glm::vec2(1, 5);
		}
		else if (point->y == 3) {
			TwAddVarRW(VolumeBar, "ColorG", TW_TYPE_TF4F, &volumeTransferFunct.rCol, " label='color' group='G' ");
			TwAddVarRW(VolumeBar, "PositionG", TW_TYPE_TF4F, &volumeTransferFunct.rPos, " label='position' group='G' ");
			volumeTransferFunct.pointNumbers[1] = glm::vec2(1, 4);
		}
	}
	if (point->x == 3) {
		volumeTransferFunct.bCol.push_back(255);
		volumeTransferFunct.bPos.push_back(1.0f);
		TwRemoveVar(VolumeBar, "ColorB");
		TwRemoveVar(VolumeBar, "PositionB");
		if (point->y == 4) {
			TwAddVarRW(VolumeBar, "ColorB", TW_TYPE_TF5F, &volumeTransferFunct.bCol, " label='color' group='B' ");
			TwAddVarRW(VolumeBar, "PositionB", TW_TYPE_TF5F, &volumeTransferFunct.bPos, " label='position' group='B' ");
			volumeTransferFunct.pointNumbers[2] = glm::vec2(1, 5);
		}
		else if (point->y == 3) {
			TwAddVarRW(VolumeBar, "ColorB", TW_TYPE_TF4F, &volumeTransferFunct.bCol, " label='color' group='B' ");
			TwAddVarRW(VolumeBar, "PositionB", TW_TYPE_TF4F, &volumeTransferFunct.bPos, " label='position' group='B' ");
			volumeTransferFunct.pointNumbers[3] = glm::vec2(1, 4);
		}
	}
	if (point->x == 4) {
		volumeTransferFunct.aCol.push_back(255);
		volumeTransferFunct.aPos.push_back(1.0f);
		TwRemoveVar(VolumeBar, "ColorA");
		TwRemoveVar(VolumeBar, "PositionA");
		if (point->y == 4) {
			TwAddVarRW(VolumeBar, "ColorA", TW_TYPE_TF5F, &volumeTransferFunct.aCol, " label='color' group='A' ");
			TwAddVarRW(VolumeBar, "PositionA", TW_TYPE_TF5F, &volumeTransferFunct.aPos, " label='position' group='A' ");
			volumeTransferFunct.pointNumbers[4] = glm::vec2(1, 5);
		}
		else if (point->y == 3) {
			TwAddVarRW(VolumeBar, "ColorA", TW_TYPE_TF4F, &volumeTransferFunct.aCol, " label='color' group='A' ");
			TwAddVarRW(VolumeBar, "PositionA", TW_TYPE_TF4F, &volumeTransferFunct.aPos, " label='position' group='A' ");
			volumeTransferFunct.pointNumbers[4] = glm::vec2(1, 4);
		}
	}
}

void TW_CALL DeletePoint(void *clientData) {
	glm::vec2 *point = static_cast<glm::vec2 *>(clientData);

	if (point->y < 3)
		return;

	if (point->x == 1) {
		volumeTransferFunct.rCol.pop_back();
		volumeTransferFunct.rPos.pop_back();
		TwRemoveVar(VolumeBar, "ColorR");
		TwRemoveVar(VolumeBar, "PositionR");
		if (point->y == 4) {
			TwAddVarRW(VolumeBar, "ColorR", TW_TYPE_TF3F, &volumeTransferFunct.rCol, " label='color' group='R' ");
			TwAddVarRW(VolumeBar, "PositionR", TW_TYPE_TF3F, &volumeTransferFunct.rPos, "label='position' group='R' ");
			volumeTransferFunct.pointNumbers[0] = glm::vec2(1, 3);
		}
		else if (point->y == 5) {
			TwAddVarRW(VolumeBar, "ColorR", TW_TYPE_TF4F, &volumeTransferFunct.rCol, " label='color' group='R' ");
			TwAddVarRW(VolumeBar, "PositionR", TW_TYPE_TF4F, &volumeTransferFunct.rPos, " label='position' group='R' ");
			volumeTransferFunct.pointNumbers[0] = glm::vec2(1, 4);
		}
	}
	if (point->x == 2) {
		volumeTransferFunct.gCol.pop_back();
		volumeTransferFunct.gPos.pop_back();
		TwRemoveVar(VolumeBar, "ColorG");
		TwRemoveVar(VolumeBar, "PositionG");
		if (point->y == 4) {
			TwAddVarRW(VolumeBar, "ColorG", TW_TYPE_TF3F, &volumeTransferFunct.rCol, " label='color' group='G' ");
			TwAddVarRW(VolumeBar, "PositionG", TW_TYPE_TF3F, &volumeTransferFunct.rPos, " label='position' group='G' ");
			volumeTransferFunct.pointNumbers[1] = glm::vec2(1, 3);
		}
		else if (point->y == 5) {
			TwAddVarRW(VolumeBar, "ColorG", TW_TYPE_TF4F, &volumeTransferFunct.rCol, " label='color' group='G' ");
			TwAddVarRW(VolumeBar, "PositionG", TW_TYPE_TF4F, &volumeTransferFunct.rPos, " label='position' group='G' ");
			volumeTransferFunct.pointNumbers[1] = glm::vec2(1, 4);
		}
	}
	if (point->x == 3) {
		volumeTransferFunct.bCol.pop_back();
		volumeTransferFunct.bPos.pop_back();
		TwRemoveVar(VolumeBar, "ColorB");
		TwRemoveVar(VolumeBar, "PositionB");
		if (point->y == 4) {
			TwAddVarRW(VolumeBar, "ColorB", TW_TYPE_TF3F, &volumeTransferFunct.bCol, " label='color' group='B' ");
			TwAddVarRW(VolumeBar, "PositionB", TW_TYPE_TF3F, &volumeTransferFunct.bPos, " label='position' group='B' ");
			volumeTransferFunct.pointNumbers[2] = glm::vec2(1, 3);
		}
		else if (point->y == 5) {
			TwAddVarRW(VolumeBar, "ColorB", TW_TYPE_TF4F, &volumeTransferFunct.bCol, " label='color' group='B' ");
			TwAddVarRW(VolumeBar, "PositionB", TW_TYPE_TF4F, &volumeTransferFunct.bPos, " label='position' group='B' ");
			volumeTransferFunct.pointNumbers[2] = glm::vec2(1, 4);
		}
	}
	if (point->x == 4) {
		volumeTransferFunct.aCol.pop_back();
		volumeTransferFunct.aPos.pop_back();
		TwRemoveVar(VolumeBar, "ColorA");
		TwRemoveVar(VolumeBar, "PositionA");
		if (point->y == 4) {
			TwAddVarRW(VolumeBar, "ColorA", TW_TYPE_TF3F, &volumeTransferFunct.aCol, " label='color' group='A' ");
			TwAddVarRW(VolumeBar, "PositionA", TW_TYPE_TF3F, &volumeTransferFunct.aPos, " label='position' group='A' ");
			volumeTransferFunct.pointNumbers[3] = glm::vec2(1, 3);
		}
		else if (point->y == 5) {
			TwAddVarRW(VolumeBar, "ColorA", TW_TYPE_TF4F, &volumeTransferFunct.aCol, " label='color' group='A' ");
			TwAddVarRW(VolumeBar, "PositionA", TW_TYPE_TF4F, &volumeTransferFunct.aPos, " label='position' group='A' ");
			volumeTransferFunct.pointNumbers[3] = glm::vec2(1, 4);
		}
	}
}

void ChangeVolume()
{
	if (m_currentMesh != m_oldMesh) {
		if (m_currentMesh == SLOT) {
			std::cout << "Two-toed Sloth" << std::endl;
			volumeTexture = volume1;
		}
		else if (m_currentMesh == CROC) {
			std::cout << "American Alligator" << std::endl;
			volumeTexture = volume2;
		}
		else if (m_currentMesh == LAMA) {
			std::cout << "Lama" << std::endl;
			volumeTexture = volume3;
		}
		else if (m_currentMesh == TURTLE) {
			std::cout << "Turtle" << std::endl;
			volumeTexture = volume5;
		}
		else if (m_currentMesh == PYTHON) {
			std::cout << "Python" << std::endl;
			volumeTexture = volume4;
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
	TwType TW_TYPE_VECTOR3F = TwDefineStruct("Vector3f", Vector3fMembers, 3, sizeof(glm::vec3), NULL, NULL);
	TwType TW_TYPE_VECTOR4F = TwDefineStruct("Vector4f", Vector4fMembers, 4, sizeof(glm::vec4), NULL, NULL);
	TW_TYPE_TF5F = TwDefineStruct("TF5F", TF5F, 5, 20, NULL, NULL);
	TW_TYPE_TF4F = TwDefineStruct("TF4F", TF4F, 4, 16, NULL, NULL);
	TW_TYPE_TF3F = TwDefineStruct("TF3F", TF3F, 3, 12, NULL, NULL);
	MeshTwType = TwDefineEnum("MeshType", Meshes, 1);

	MeshBar = TwNewBar("Mesh");
	TwAddVarRW(MeshBar, "Mesh", MeshTwType, &m_currentMesh, NULL);
	TwAddVarRW(MeshBar, "Slice", TW_TYPE_INT32, &numPlanes, " label='# slices'  ");
	TwAddButton(MeshBar, "Load additional meshes", LoadMeshes, NULL, " help='This will take some time' ");
	TwAddSeparator(MeshBar, "", NULL);	
	TwAddVarRW(MeshBar, "Specular Light", TW_TYPE_BOOLCPP, &useSpecularLight, " label='Specular Comp.' group='Light'");
	TwAddVarRW(MeshBar, "Spec. Exp.", TW_TYPE_FLOAT, &shininess, " label='Spec. Exp.' group='Light' step=0.1");
	TwAddVarRW(MeshBar, "Intensity Correction", TW_TYPE_BOOLCPP, &useIi, " label='Intensity Cor.' group='Light'");
	TwAddVarRW(MeshBar, "View Ray Refraction", TW_TYPE_BOOLCPP, &useVRayRefraction, " label='View Ref.' group='Light'");
	TwAddSeparator(MeshBar, "", NULL);

	/*glm::vec3 camPos = camera.getPosition();
	glm::vec3 camDir = camera.getFront();
	TwAddVarRO(MeshBar, "pos", TW_TYPE_VECTOR3F, &camPos, " label='Position' group='Camera'");
	TwAddVarRO(MeshBar, "dir", TW_TYPE_VECTOR3F, &camDir, " label='Direction' group='Camera'");*/
	TwAddButton(MeshBar, "Reset", ResetCamera, NULL, " group='Camera' ");
	TwAddVarRW(MeshBar, "Camera Speed", TW_TYPE_FLOAT, &cameraPointer->speed, " min=0.1 max=20.0 step=0.1 group='Camera' ");
	TwAddVarRW(MeshBar, "Rotate", TW_TYPE_BOOL8, &autoRotate, " group='Camera' " );
	TwDefine(" Mesh/Camera opened=true ");
	TwAddSeparator(MeshBar, "", NULL);
	TwDefine(" Mesh label='Mesh' refresh=0.5 size='200 270' position='10 10' iconpos=bottomleft iconalign=vertical");

	VolumeBar = TwNewBar("VolumeTFBar");
	TwAddButton(VolumeBar, "AddPointR", AddPoint, &volumeTransferFunct.pointNumbers[0], "group = 'R' label='Add point'");
	TwAddButton(VolumeBar, "RemovePointR", DeletePoint, &volumeTransferFunct.pointNumbers[0], "group ='R' label='Remove point'");
	TwAddVarRW(VolumeBar, "ColorR", TW_TYPE_TF4F, &volumeTransferFunct.rCol, "group='R' label='color'");
	TwAddVarRW(VolumeBar, "PositionR", TW_TYPE_TF4F, &volumeTransferFunct.rPos, "group='R' label='position'");
	TwDefine(" VolumeTFBar/R opened=false ");
	TwAddButton(VolumeBar, "AddPointG", AddPoint, &volumeTransferFunct.pointNumbers[1], "group = 'G' label='Add point'");
	TwAddButton(VolumeBar, "RemovePointG", DeletePoint, &volumeTransferFunct.pointNumbers[1], "group ='G' label='Remove point'");
	TwAddVarRW(VolumeBar, "ColorG", TW_TYPE_TF5F, &volumeTransferFunct.gCol, "group='G' label='color' ");
	TwAddVarRW(VolumeBar, "PositionG", TW_TYPE_TF5F, &volumeTransferFunct.gPos, "group='G' label='position'");
	TwDefine(" VolumeTFBar/G opened=false ");
	TwAddButton(VolumeBar, "AddPointB", AddPoint, &volumeTransferFunct.pointNumbers[2], "group = 'B' label='Add point'");
	TwAddButton(VolumeBar, "RemovePointB", DeletePoint, &volumeTransferFunct.pointNumbers[2], "group ='B' label='Remove point'");
	TwAddVarRW(VolumeBar, "ColorB", TW_TYPE_TF3F, &volumeTransferFunct.bCol, "group='B' label='color' ");
	TwAddVarRW(VolumeBar, "PositionB", TW_TYPE_TF3F, &volumeTransferFunct.bPos, "group='B' label='position'");
	TwDefine(" VolumeTFBar/B opened=false ");
	TwAddButton(VolumeBar, "AddPointA", AddPoint, &volumeTransferFunct.pointNumbers[3], "group = 'A' label='Add point'");
	TwAddButton(VolumeBar, "RemovePointA", DeletePoint, &volumeTransferFunct.pointNumbers[3], "group ='A' label='Remove point'");
	TwAddVarRW(VolumeBar, "ColorA", TW_TYPE_TF3F, &volumeTransferFunct.aCol, "group='A' label='color' ");
	TwAddVarRW(VolumeBar, "PositionA", TW_TYPE_TF3F, &volumeTransferFunct.aPos, "group='A' label='position' ");
	TwDefine(" VolumeTFBar/A opened=false ");
	TwAddButton(VolumeBar, "Apply Volume TF", ApplyVolTransferChanges, &volumeTransferFunct, NULL);
	TwAddButton(VolumeBar, "Reset Volume TF", ResetVolTransferFunction, &volumeTransferFunct, NULL);

	TwDefine(" VolumeTFBar label='Volume TF' refresh=0.5 size='200 270' position='655 10' iconpos=bottomleft iconalign=vertical");

	/*MediumBar = TwNewBar("MediumTFBar");
	TwAddVarRW(MediumBar, "tf1colorm", TW_TYPE_COLOR4F, &mediumTransferFunct.color1, " label='TF1'");
	TwAddVarRW(MediumBar, "tf1posm", TW_TYPE_FLOAT, &mediumTransferFunct.position.x, " label='TF1 position' min=0 max=1 step=0.1");
	TwAddVarRW(MediumBar, "tf2colorm", TW_TYPE_COLOR4F, &mediumTransferFunct.color2, " label='TF2'");
	TwAddVarRW(MediumBar, "tf2posm", TW_TYPE_FLOAT, &mediumTransferFunct.position.y, " label='TF2 position' min=0 max=1 step=0.1");
	TwAddVarRW(MediumBar, "tf3colorm", TW_TYPE_COLOR4F, &mediumTransferFunct.color3, " label='TF3'");
	TwAddVarRW(MediumBar, "tf3posm", TW_TYPE_FLOAT, &mediumTransferFunct.position.z, " label='TF3 position' min=0 max=1 step=0.1");
	TwAddVarRW(MediumBar, "tf4colorm", TW_TYPE_COLOR4F, &mediumTransferFunct.color4, " label='TF4'");
	TwAddVarRW(MediumBar, "tf4posm", TW_TYPE_FLOAT, &mediumTransferFunct.position.w, " label='TF4 position' min=0 max=1 step=0.1");
	TwAddButton(MediumBar, "Apply Medium TF", ApplyMedTransferChanges2, &mediumTransferFunct, NULL);
	TwAddButton(MediumBar, "Reset Medium TF", ResetMedTransferFunction, &mediumTransferFunct, NULL);
	TwDefine(" MediumTFBar label='Medium TF' refresh=0.5 size='200 270' position='655 295' iconpos=bottomleft iconalign=vertical");
*/
}
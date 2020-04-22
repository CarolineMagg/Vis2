#include <GL\glew.h> 
#include <GLFW\glfw3.h>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\string_cast.hpp>

#include "camera\Camera.h"
#include "tex/Texture.h"
#include "mesh/Cube.h"
#include "render/FBO.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <locale>
#include <algorithm>
#include <memory>

#include "shading/Shader.h"

unsigned int WIDTH_DEFAULT = 800;
unsigned int HEIGHT_DEFAULT = 600;

#define EXIT_WITH_ERROR(err) \
	std::cout << "ERROR: " << err << std::endl; \
	system("PAUSE"); \
	return EXIT_FAILURE;

// set parameters
float fov = 85.0f;
int fps = 200;
float frameDeltaTime = 0;
bool isFullScreen = false;
bool wireframe = false;
float brightness = 0.00f;

// create objects
GLFWwindow* window;
Camera* cameraPointer;

void renderQuad();

// method init
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Camera& camera);
void processMouseInput(GLFWwindow* window, double xpos, double ypos);
void processScrollInput(GLFWwindow* window, double xoffset, double yoffset);
void processKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods);

static void APIENTRY DebugCallbackDefault(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
static void trim(std::string& string);


int main()
{	
	// init GLFW and return failed
	if (!glfwInit()) {
		printf("failed to initialize GLFW.\n");
		return -1;
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
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// initialize GLEW
	glewExperimental = true;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		EXIT_WITH_ERROR("Failed to init GLEW")
	}	
	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	// enable OpenGL parameters
	glViewport(0, 0, WIDTH_DEFAULT, HEIGHT_DEFAULT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);  //MSAA
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);	
	glEnable(GL_TEXTURE_3D);

	// set window parameters 
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //no cursor
	glfwSetCursorPosCallback(window, processMouseInput);
	glfwSetScrollCallback(window, processScrollInput);
	glfwSetKeyCallback(window, processKeyboardInput);
	if (!isFullScreen) {
		glfwSetWindowMonitor(window, NULL, 0, 0, WIDTH_DEFAULT, HEIGHT_DEFAULT, fps);
	}
	else {
		glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, WIDTH_DEFAULT, HEIGHT_DEFAULT, fps);
	}

#if _DEBUG
	//Register Callback function
	glDebugMessageCallback(DebugCallbackDefault, NULL);
	//Synchronuous Callback - immediatly after error
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

	// reset color/ texture bindings
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// setup camera
	Camera camera(fov, (float)(WIDTH_DEFAULT) / HEIGHT_DEFAULT, 0.1f, 120.0f);
	cameraPointer = &camera;

	// this is for drawing random squares
#ifdef _DEBUG
	Shader debugDrawShader("debug_texturedraw_vs.txt", "debug_texturedraw_fs.txt");
#endif

	// init shaders
	Shader basicRender("basic_render_vs.txt", "basic_render_fs.txt");
	Shader volumeRender("vol_vs.txt", "vol_fs.txt");

	// setup scene
	printf("Loading assets...\n");
	Texture tex;	
	int volumeTexture = tex.load3DTexture("assets/images/volumeData/Choloepis_hoffmani/PNG/CHOL", 512, 512, 1, 1, 441, ".png", 4);		
	//to test single images: 
	//Texture tex2{ "CHOL0001.png","assets/images/volumeData/Choloepis_hoffmani/PNG", "test"};
		
	// this is our "virtual" room. we render this once per frame in background,
	// so that we get the depth of our scene and march back from those fragments
	Cube virtualRoom{2.f, 2.f, 2.f};
	glm::mat4 virtualRoomModelM;	
	FBO roomMap{ false, true, false, WIDTH_DEFAULT, HEIGHT_DEFAULT };

	printf("Loading assets finished...\n");

	// init camera position and frame counter
	double xpos = 0;
	double ypos = 0;
	double fps_timer = 0;
	int fps_counter = 0;
	float lastFrame = (float)glfwGetTime();

	// render loop
	while (!glfwWindowShouldClose(window)) {

		// per-frame time
		float currentFrame = (float)glfwGetTime();
		frameDeltaTime = currentFrame - lastFrame;
		fps_timer += frameDeltaTime;
		++fps_counter;
		if (fps_timer > 1.0f) {
			std::cout << float(fps_counter) / fps_timer << std::endl;			
			fps_timer = 0;
			fps_counter = 0;
		}
		lastFrame = currentFrame;		

		// update camera
		glfwGetCursorPos(window, &xpos, &ypos);
		camera.processMouseInput(window, xpos, ypos, frameDeltaTime);
		camera.processKeyInput(window, frameDeltaTime);
		camera.frameUpdate(frameDeltaTime);
		const glm::mat4& camView = camera.getViewMatrix();

		// render clear
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);		
		glViewport(0, 0, WIDTH_DEFAULT, HEIGHT_DEFAULT);		
				
		// render our room, but only the "back" side of the cube since we are
		// only interested in "inner walls" and don't wanna see the outer ones
		// this cube is rendered to get a depth map of the render area
		roomMap.setActive();		
		basicRender.use();
		basicRender.setUniform("modelMatrix", virtualRoomModelM);
		basicRender.setUniform("viewProjMatrix", camera.getProjection() * camView);
		glCullFace(GL_FRONT);
		virtualRoom.draw();
		glCullFace(GL_BACK);

		// set back to default frame buffer from roomMap
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/// VOLUME RENDER: bind textures and call render Quad////////////
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		volumeRender.use();
		volumeRender.setUniform("inverseView", glm::inverse(camView));
		volumeRender.setUniform("inverseProjection", glm::inverse(camera.getProjection()));
		volumeRender.setUniform("viewPos", camera.getPosition());		

		glActiveTexture(GL_TEXTURE0 + 0);
		volumeRender.setUniform("sceneDepth", 0);
		glBindTexture(GL_TEXTURE_2D, roomMap.getDepthTexture());

		glActiveTexture(GL_TEXTURE0 + 1);
		volumeRender.setUniform("volTexture", 1);
		glBindTexture(GL_TEXTURE_3D, volumeTexture);	

		renderQuad();

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

void processInput(GLFWwindow *window, Camera& camera) {
	//camera.processKeyInput(window, frameDeltaTime);
}

void processKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// ESCAPE - close window
	// F11 - full screen
	// Space - debug information of camera
	// WASD - camera movement forward, left, backwards, right

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}	
	else if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {

		if (isFullScreen) {
			glfwSetWindowMonitor(window, NULL, 0, 0, WIDTH_DEFAULT, HEIGHT_DEFAULT, fps);
			isFullScreen = false;
		}
		else {
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, WIDTH_DEFAULT, HEIGHT_DEFAULT, fps);
			isFullScreen = true;
		}
	}
	else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {

		std::cout << "Position: " << cameraPointer->getPosition().x << "," << cameraPointer->getPosition().y << "," << cameraPointer->getPosition().z << std::endl;
		std::cout << "Direction: " << cameraPointer->getFront().x << "," << cameraPointer->getFront().y << "," << cameraPointer->getFront().z << std::endl;
	}
}

void processMouseInput(GLFWwindow* window, double xpos, double ypos) {

}
void processScrollInput(GLFWwindow* window, double xoffset, double yoffset) {
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= (float)yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 100.0f)
		fov = 100.0f;
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

// TODO: do we need this?
// trim from start (in place)
static inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
	}));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}
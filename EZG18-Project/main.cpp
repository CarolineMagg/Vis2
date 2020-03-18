#include <gl3w.h>
#include <GLFW\glfw3.h>
#include <glm\gtc\matrix_transform.hpp>
#include <algorithm>
#include <memory>

#include "shading\Shader.h"
#include "camera\Camera.h"
#include "mesh\Model.h"
#include "lights\Light.h"
#include "Scene.h"
#include "SceneBuilder\SceneBuilder.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <locale>

float fov = 85.0f;
int fps = 200;
float frameDeltaTime = 0;
bool isFullScreen = false;
bool wireframe = false;
float brightness = 0.00f;

glm::mat4 projection;

GLFWwindow* window;
Camera* cameraPointer;
std::unique_ptr<Scene> scene;

void renderQuad();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, Camera& camera);
void processMouseInput(GLFWwindow* window, double xpos, double ypos);
void processScrollInput(GLFWwindow* window, double xoffset, double yoffset);
void processKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods);

static void APIENTRY DebugCallbackDefault(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
static void trim(std::string& string);

int main() {

	//load texture config
	std::ifstream configFile;
	configFile.open("assets/config.ini");

	//line string
	std::string line;
	if (configFile.is_open())
	{
		while (std::getline(configFile, line))
		{
			trim(line);
			std::istringstream configLine(line);
			std::string key;
			if (std::getline(configLine, key, '=')) {
				trim(key);
				std::string value;
				if (std::getline(configLine, value)) {
					trim(value);
					if (key == "fullscreen")
					{
						std::istringstream(value) >> std::boolalpha >> isFullScreen;
					}
					if (key == "width")
					{
						Scene::WIDTH_DEFAULT = std::stoi(value);
					}
					if (key == "height")
					{
						Scene::HEIGHT_DEFAULT = std::stoi(value);
					}
					if (key == "MSAA")
					{
						FBO::MULTI_SAMPLE_COUNT = std::stoi(value);
					}
				}
			}
		}
	}


	if (!glfwInit()) {
		printf("failed to initialize GLFW.\n");
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, FBO::MULTI_SAMPLE_COUNT);
	glfwWindowHint(GLFW_REFRESH_RATE, fps);
#if _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	window = glfwCreateWindow(Scene::WIDTH_DEFAULT, Scene::HEIGHT_DEFAULT, "EZG-2018", nullptr, nullptr);
	if (!window) {
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	if (gl3wInit()) {
		printf("failed to initialize OpenGL\n");
		return -1;
	}
	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	glViewport(0, 0, Scene::WIDTH_DEFAULT, Scene::HEIGHT_DEFAULT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);  //MSAA
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //no cursor
	glfwSetCursorPosCallback(window, processMouseInput);
	glfwSetScrollCallback(window, processScrollInput);
	glfwSetKeyCallback(window, processKeyboardInput);

	//set fullscreen
	if (!isFullScreen) {
		glfwSetWindowMonitor(window, NULL, 0, 0, Scene::WIDTH_DEFAULT, Scene::HEIGHT_DEFAULT, fps);
	} else {
		glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, Scene::WIDTH_DEFAULT, Scene::HEIGHT_DEFAULT, fps);
	}

#if _DEBUG
	//Register Callback function
	glDebugMessageCallback(DebugCallbackDefault, NULL);
	//Synchronuous Callback - immediatly after error
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	//BIRGHTNESS////////////////////////////////////////////////////  
	float brightnessVertices[] = {
		1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f
	};
	unsigned int brightnessIndices[] = {
		0, 3, 1,
		1, 3, 2
	};
	unsigned int bVBO, bVAO, bEBO;
	glGenVertexArrays(1, &bVAO);
	glGenBuffers(1, &bVBO);
	glGenBuffers(1, &bEBO);
	glBindVertexArray(bVAO);
	glBindBuffer(GL_ARRAY_BUFFER, bVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(brightnessVertices), brightnessVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(brightnessVertices), brightnessIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	Shader brightnessShader("brightness_vs.txt", "brightness_fs.txt");
	//////////////////////////////////////////// end brightnss
	   
	//camera
	Camera camera(fov, (float)(Scene::WIDTH_DEFAULT) / Scene::HEIGHT_DEFAULT, 0.1f, 120.0f);
	cameraPointer = &camera;

#ifdef _DEBUG
	Shader debugDrawShader("debug_texturedraw_vs.txt", "debug_texturedraw_fs.txt");
#endif

	//Setup Scene
	printf("Loading assets...\n");
	scene = SceneBuilder::buildScene(camera);
	scene->msTest = FBO::MULTI_SAMPLE_COUNT;
	printf("Loading assets finished...\n");

	double xpos = 0;
	double ypos = 0;
	double fps_timer = 0;
	int fps_counter = 0;
	// render loop
	float lastFrame = (float)glfwGetTime();
	while (!glfwWindowShouldClose(window)) {

		// per-frame time
		float currentFrame = (float)glfwGetTime();
		frameDeltaTime = currentFrame - lastFrame;
		fps_timer += frameDeltaTime;
		++fps_counter;
		if (fps_timer > 1.0f) {
			std::cout << float(fps_counter)/ fps_timer << std::endl;
			fps_timer = 0;
			fps_counter = 0;
		}

		lastFrame = currentFrame;

		//update camera
		glfwGetCursorPos(window, &xpos, &ypos);
		camera.processMouseInput(window, xpos, ypos, frameDeltaTime);
		processInput(window, camera);
		camera.frameUpdate(frameDeltaTime);

		//render
		scene->preRender(frameDeltaTime);
		glViewport(0, 0, Scene::WIDTH_DEFAULT, Scene::HEIGHT_DEFAULT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		scene->render();

#ifdef _DEBUG
		//debugDrawShader.use();
		//debugDrawShader.setUniform("near_plane", 0.5f);
		//debugDrawShader.setUniform("far_plane", 50.0f);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, Scene::txtest);
		//renderQuad();
#endif
		// "draw" brightness (no gamma)
		if (brightness >= 0.01f) {
			brightnessShader.use();
			brightnessShader.setUniform("brightness", brightness);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBindVertexArray(bVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			renderQuad();
		}

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

void processInput(GLFWwindow *window, Camera& camera) {
	camera.processKeyInput(window, frameDeltaTime);
}

void processKeyboardInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
		cameraPointer->locked = !(cameraPointer->locked);
	} else if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
		if (scene->msTest == 12) scene->msTest = 16;
		else if (scene->msTest == 16) scene->msTest = 12;
	} else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	else if (key == GLFW_KEY_F6 && action == GLFW_PRESS) {
		brightness = std::min(0.2f, brightness + 0.01f);
		std::cout << "Brightness: " << brightness << std::endl;
	} else if (key == GLFW_KEY_F7 && action == GLFW_PRESS) {
		brightness = std::max(0.0f, brightness - 0.01f);
		std::cout << "Brightness: " << brightness << std::endl;
	} else if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {

		if (isFullScreen) {
			glfwSetWindowMonitor(window, NULL, 0, 0, Scene::WIDTH_DEFAULT, Scene::HEIGHT_DEFAULT, fps);
			isFullScreen = false;
		} else {
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, Scene::WIDTH_DEFAULT, Scene::HEIGHT_DEFAULT, fps);
			isFullScreen = true;
		}
	} else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {

		 std::cout << "Position: " << cameraPointer->getPosition().x << ","<< cameraPointer->getPosition().y << "," << cameraPointer->getPosition().z << std::endl;
		 std::cout << "Direction: " << cameraPointer->getFront().x << ","<< cameraPointer->getFront().y << "," << cameraPointer->getFront().z << std::endl;
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
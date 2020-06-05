#include <GL\glew.h> 
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include "Shader.h"

unsigned int Shader::currentGLShader = 0;
const std::string Shader::SHADER_DIR = "assets/shaders/";

const unsigned int Shader::SPOT_COUNT = 1;
const unsigned int Shader::POINT_COUNT = 2;

Shader::Shader(const std::string& vertexFile, const std::string& fragmentPath) {
		compileVertFragShader(vertexFile, fragmentPath);		
}

Shader::Shader(const std::string& vertexFile, const std::string& fragmentFile, const std::string& geometryFile, bool setInterleaved, const std::vector<char>* varys) {
		compileVertFragGeomShader(vertexFile, fragmentFile, geometryFile, setInterleaved, varys);
}

Shader::Shader(const std::string& computeShaderFile)
{
	std::string computeCode;	
	std::ifstream cShaderFile;	
	// enable exc
	cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	
	try {
		cShaderFile.open(SHADER_DIR + computeShaderFile);
		std::stringstream cShaderStream;
		cShaderStream << cShaderFile.rdbuf();		
		cShaderFile.close();		
		computeCode = cShaderStream.str();		
	}
	catch (std::ifstream::failure e) {
		std::cout << "SHADER FILE ERROR::COULD NOT READ FILE" << std::endl;
	}

	const char* cShaderCode = computeCode.c_str();

	unsigned int compute;
	//vertex shader
	compute = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute, 1, &cShaderCode, NULL);
	glCompileShader(compute);
	checkCompileErrors(compute, "COMPUTE");

	ID = glCreateProgram();
	glAttachShader(ID, compute);	
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
	buildUniformTable();
	glDeleteShader(compute);	
}

void Shader::compileVertFragShader(const std::string& vertexFile, const std::string& fragmentFile) {
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	// enable exc
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		vShaderFile.open(SHADER_DIR + vertexFile);
		fShaderFile.open(SHADER_DIR + fragmentFile);
		std::stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		vShaderFile.close();
		fShaderFile.close();
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	} catch (std::ifstream::failure e) {
		std::cout << "SHADER FILE ERROR::COULD NOT READ FILE" << std::endl;
	}
	
	replaceNumConstants(vertexCode, "**%%SPOT_LIGHT_COUNT%%**", SPOT_COUNT);
	replaceNumConstants(vertexCode, "**%%POINT_LIGHT_COUNT%%**", POINT_COUNT);
	replaceNumConstants(fragmentCode, "**%%SPOT_LIGHT_COUNT%%**", SPOT_COUNT);
	replaceNumConstants(fragmentCode, "**%%POINT_LIGHT_COUNT%%**", POINT_COUNT);

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	unsigned int vertex, fragment;
	//vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	//fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
	buildUniformTable();
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::compileVertFragGeomShader(const std::string& vertexFile, const std::string& fragmentFile, const std::string& geometryFile, bool setInterleaved, const std::vector<char>* varys) {
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	// enable exc
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		vShaderFile.open(SHADER_DIR + vertexFile);
		fShaderFile.open(SHADER_DIR + fragmentFile);
		gShaderFile.open(SHADER_DIR + geometryFile);
		std::stringstream vShaderStream, fShaderStream, gShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		gShaderStream << gShaderFile.rdbuf();
		vShaderFile.close();
		fShaderFile.close();
		gShaderFile.close();
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		geometryCode = gShaderStream.str();
	} catch (std::ifstream::failure e) {
		std::cout << "SHADER FILE ERROR::COULD NOT READ FILE" << std::endl;
	}
	
	replaceNumConstants(vertexCode, "**%%SPOT_LIGHT_COUNT%%**", SPOT_COUNT);
	replaceNumConstants(vertexCode, "**%%POINT_LIGHT_COUNT%%**", POINT_COUNT);
	replaceNumConstants(fragmentCode, "**%%SPOT_LIGHT_COUNT%%**", SPOT_COUNT);
	replaceNumConstants(fragmentCode, "**%%POINT_LIGHT_COUNT%%**", POINT_COUNT);
	replaceNumConstants(geometryCode, "**%%SPOT_LIGHT_COUNT%%**", SPOT_COUNT);
	replaceNumConstants(geometryCode, "**%%POINT_LIGHT_COUNT%%**", POINT_COUNT);

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	const char* gShaderCode = geometryCode.c_str();


	unsigned int vertex, fragment, geometry;
	//vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	//geometry Shader
	geometry = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(geometry, 1, &gShaderCode, NULL);
	glCompileShader(geometry);
	checkCompileErrors(geometry, "GEOMETRY");
	//fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glAttachShader(ID, geometry);

	if (setInterleaved) {
		//TODO: fix this mess
		const GLchar* varys2[4];
		varys2[0] = "T";
		varys2[1] = "P";
		varys2[2] = "V";
		varys2[3] = "A";		
		glTransformFeedbackVaryings(ID, 4, varys2, GL_INTERLEAVED_ATTRIBS);		
	}
	
	glLinkProgram(ID);

	checkCompileErrors(ID, "PROGRAM");	

	buildUniformTable();
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	glDeleteShader(geometry);
}

void Shader::link() {
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
}

void Shader::use() const {
	if (currentGLShader == ID) return;
	currentGLShader = ID;
	glUseProgram(ID);
}


// utility uniform functions
// ------------------------------------------------------------------------
void Shader::setUniform(const std::string &name, bool value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setUniform(const std::string &name, int value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setUniform(const std::string &name, unsigned int value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setUniform(const std::string &name, float value) const {
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setUniform(const std::string &name, double value) const {
	glUniform1d(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setUniform(const std::string &name, float value1, float value2, float value3, float value4) const {
	glUniform4f(glGetUniformLocation(ID, name.c_str()), value1, value2, value3, value4);
}
//
void Shader::setUniform(const std::string& name, const glm::ivec2& value) const {
	glUniform2iv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setUniform(const std::string &name, const glm::vec2 &value) const {
	glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setUniform(const std::string &name, float x, float y) const {
	glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
// ------------------------------------------------------------------------
void Shader::setUniform(const std::string &name, const glm::vec3 &value) const {
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setUniform(const std::string &name, float x, float y, float z) const {
	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::setUniform(const std::string &name, const glm::vec4 &value) const {
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
// ------------------------------------------------------------------------
void Shader::setUniform(const std::string &name, const glm::ivec4 &value) const {
	glUniform4iv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
// ------------------------------------------------------------------------
void Shader::setUniform(const std::string &name, const glm::mat2 &mat) const {
	glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setUniform(const std::string &name, const glm::mat3 &mat) const {
	glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setUniform(const std::string &name, const glm::mat4 &mat) const {
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::checkCompileErrors(unsigned int shader, std::string type) {
	int success;
	char infoLog[2048];
	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR: " << type << "\n" << infoLog << std::endl;
		}
	} else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "LINKING_ERROR PROGRAM: " << ID << "\n" << infoLog << std::endl;
		} else {
			//std::cout << "Shader program " << ID << " linked." << std::endl;
		}
	}
}

GLint Shader::getUniformLocation(const std::string name) const
{
	auto search = locations.find(name);

	if (search != locations.end())
	{
		return search->second;
	}
	else
	{
		return -1;
	}
}


void Shader::buildUniformTable()
{
	GLint i;
	GLint count;

	GLint size; // size of the variable
	GLenum type; // type of the variable (float, vec3 or mat4, etc)

	const GLsizei bufSize=128; // maximum name length

	GLchar name[bufSize]; // variable name in GLSL
	GLsizei length; // name length

	glGetProgramiv(ID, GL_ACTIVE_UNIFORMS, &count);

	//printf("Active Uniforms: %d\n", count);

	for (i = 0; i < count; i++)
	{
		glGetActiveUniform(ID, (GLuint)i, bufSize, &length, &size, &type, name);

		//printf("Uniform #%d Type: %u Name: %s Size %i\n" , i, type, name, size);

		std::string nameString(name);
		std::size_t found = nameString.find("[0]");

		for(int i = 0; i < size; ++i)
		{
			if (found != std::string::npos) {
				nameString.replace(found, 3, "[" + std::to_string(i) + "]");
			}
			GLint location = glGetUniformLocation(ID, nameString.c_str());
			//::cout << nameString << std::endl;
			locations.insert({ nameString, location });
		}
	}
	//std::cout << "finished" << std::endl;
}

void Shader::replaceNumConstants(std::string& str, const std::string& from, const unsigned int to) {
	size_t start_pos = 0;
	std::string to_s = std::to_string(to);
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to_s);
		start_pos += to_s.length();
	}	
}
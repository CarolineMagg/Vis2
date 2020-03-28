#pragma once
#include <glm\glm.hpp>
#include <unordered_map>
#include <string>


class Shader {
public:
	//program ID
	unsigned int ID;	
	Shader(const std::string& vertexFile, const std::string& fragmentFile);
	Shader(const std::string& vertexFile, const std::string& fragmentFile, const std::string& geometryFile, bool setInterleaved = false, const std::vector<char>* varys = nullptr);
	void use() const;
	void link();
	
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setUniform(const std::string &name, bool value) const;
	void setUniform(const std::string &name, int value) const;
	void setUniform(const std::string &name, unsigned int value) const;
	void setUniform(const std::string &name, float value) const;
	void setUniform(const std::string &name, double value) const;
	void setUniform(const std::string &name, float value1, float value2, float value3, float value4) const;
	void setUniform(const std::string &name, const glm::vec2 &value) const;
	void setUniform(const std::string &name, float x, float y) const;
	void setUniform(const std::string &name, const glm::vec3 &value) const;
	void setUniform(const std::string &name, float x, float y, float z) const;
	void setUniform(const std::string &name, const glm::vec4 &value) const;
	void setUniform(const std::string &name, const glm::ivec4 &value) const;	
	void setUniform(const std::string &name, const glm::mat2 &mat) const;
	void setUniform(const std::string &name, const glm::mat3 &mat) const;
	void setUniform(const std::string &name, const glm::mat4 &mat) const;

private:
	static unsigned int currentGLShader;
	static void replaceNumConstants(std::string& str, const std::string& from, const unsigned int to);
	// utility function for checking shader compilation/linking errors.
	void checkCompileErrors(unsigned int shader, std::string type);	
	//Uniform table for faster lookup (a lot of cost in asking gpu)
	std::unordered_map<std::string, int> locations;
	int getUniformLocation(const std::string name) const;
	void buildUniformTable();	

protected:	

	static const std::string SHADER_DIR;
	static const unsigned int SPOT_COUNT;
	static const unsigned int POINT_COUNT;

	void compileVertFragShader(const std::string& vertexFile, const std::string& fragmentFile);
	void compileVertFragGeomShader(const std::string& vertexFile, const std::string& fragmentFile, const std::string& geometryFile, bool setInterleaved = false, const std::vector<char>* varys = nullptr);
};


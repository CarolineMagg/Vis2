#pragma once
#include "GL/glew.h"
#include <glm\gtc\matrix_transform.hpp>
#include <vector>
struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

class Cube {
public:
	Cube(const float width, const float height, const float depth);
	void draw();	
private:
	unsigned int VAO, VBO, EBO;
	unsigned int indicesCount = 0;
};



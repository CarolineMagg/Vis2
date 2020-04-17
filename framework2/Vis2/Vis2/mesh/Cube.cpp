#include "Cube.h"
#include "GL/glew.h"
#include <glm\gtc\matrix_transform.hpp>
#include <vector>



Cube::Cube(const float width, const float height, const float depth) {
	using namespace glm;
	using namespace std;
	float vX = width / 2;
	float vY = height / 2;
	float vZ = depth / 2;

	vector<vec3> vertices;
	vertices.push_back(vec3{ -vX, vY, vZ });		//0
	vertices.push_back(vec3{ -vX, -vY, vZ });		//1
	vertices.push_back(vec3{ vX, vY, vZ });			//2
	vertices.push_back(vec3{ vX, -vY, vZ });		//3

	vertices.push_back(vec3{ -vX, vY, -vZ });		//4
	vertices.push_back(vec3{ -vX, -vY, -vZ });		//5
	vertices.push_back(vec3{ vX, vY, -vZ });		//6
	vertices.push_back(vec3{ vX, -vY, -vZ });		//7

	unsigned int indices[] = {
		0, 1, 2,	2, 1, 3,
		4, 5, 0,	0, 5, 1,
		4, 0, 6,	6, 0, 2,
		2, 3, 6,	6, 3, 7,
		1, 5, 3,	3, 5, 7,
		6, 7, 4,	4, 7, 5
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,  sizeof(indices), &indices[0], GL_STATIC_DRAW);

	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (GLvoid*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	indicesCount = sizeof(indices) / sizeof(unsigned int);
}

void Cube::draw() {
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
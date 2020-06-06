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

	vertices.push_back(vec3{ -vX, vY, vZ });		//0
	vertices.push_back(vec3{ -vX, -vY, vZ });		//1
	vertices.push_back(vec3{ vX, vY, vZ });			//2
	vertices.push_back(vec3{ vX, -vY, vZ });		//3

	vertices.push_back(vec3{ -vX, vY, -vZ });		//4
	vertices.push_back(vec3{ -vX, -vY, -vZ });		//5
	vertices.push_back(vec3{ vX, vY, -vZ });		//6
	vertices.push_back(vec3{ vX, -vY, -vZ });		//7

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
		6, 7, 4,	4, 7, 5,
		12, 13, 8,	8, 13, 9, // left
		20, 16, 22,	22, 16, 18, // top
		10, 11, 14,	14, 11, 15, // right
		17, 21, 19,	19, 21, 23, // bottom		
	};
	indicesCount = sizeof(indices) / sizeof(unsigned int);

	vector<vec3> normals(vertices.size());
	for (int i = 0; i < indicesCount; i = i + 3) {
		vec3 normal = normalize(cross(vertices[indices[i + 1]] - vertices[indices[i]], vertices[indices[i + 2]] - vertices[indices[i]]));
		for (int j = 0; j < 3; j++) {
			normals[indices[i + j]] = normal;
		}
	}

	vector<vec3> output;
	for (int i = 0; i < vertices.size(); i++) {
		output.push_back(vertices[i]);
		output.push_back(normals[i]);
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, output.size() * sizeof(vec3), &output[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (GLvoid*)0);
	// normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (GLvoid*)(3 * sizeof(float)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Cube::draw() {
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
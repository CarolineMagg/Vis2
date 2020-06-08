#include "Cube.h"



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

	vector<vec2> texCoords(vertices.size());

	for (unsigned int i = 0; i < indicesCount; i = i + 6) {
		texCoords[indices[i + 0]] = vec2{ 0, 1 };
		texCoords[indices[i + 1]] = vec2{ 0, 0 };
		texCoords[indices[i + 2]] = vec2{ 1, 1 };
		texCoords[indices[i + 5]] = vec2{ 1, 0 };
	}

	vector<vec3> normals(vertices.size());
	for (unsigned int i = 0; i < indicesCount; i = i + 3) {
		vec3 normal = normalize(cross(vertices[indices[i + 1]] - vertices[indices[i]], vertices[indices[i + 2]] - vertices[indices[i]]));
		for (unsigned int j = 0; j < 3; j++) {
			normals[indices[i + j]] = normal;
		}
	}

	vector<Vertex> output;
	for (unsigned int i = 0; i < vertices.size(); i++) {
		output.push_back(Vertex{ vertices[i], normals[i], texCoords[i] });
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &output[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Cube::draw() {
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
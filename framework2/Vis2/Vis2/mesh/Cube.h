#pragma once
class Cube {
public:
	Cube(const float width, const float height, const float depth);
	void draw();
private:
	unsigned int VAO, VBO, EBO;
	unsigned int indicesCount = 0;
};


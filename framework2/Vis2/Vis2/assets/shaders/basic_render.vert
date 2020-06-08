#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 position_model;
out vec3 Normal;
out vec2 TexCoords;

out vec3 FragPos;
uniform mat4 modelMatrix;
uniform mat4 viewProjMatrix;

void main() {	
	Normal = normalize(transpose(inverse(mat3(modelMatrix))) * aNormal);
	TexCoords = aTexCoords;
	position_model = aPos;
	FragPos = vec3(modelMatrix * vec4(aPos, 1.0));
	gl_Position = viewProjMatrix * modelMatrix * vec4(aPos, 1);
}
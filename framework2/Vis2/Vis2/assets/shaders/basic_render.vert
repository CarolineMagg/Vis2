#version 430 core

layout(location = 0) in vec3 position;

out VertexData {
	vec3 position_model;	
} vert;

uniform mat4 modelMatrix;
uniform mat4 viewProjMatrix;

void main() {
	gl_Position = viewProjMatrix * modelMatrix * vec4(position, 1);
	vert.position_model = position;
}
#version 430 core

in VertexData {
	vec3 position_model;	
} vert;

out vec4 color;

uniform samplerCube tex;


void main() {
	color = vec4(0.0, 0.0, 0.0, 1) + texture(tex, vert.position_model);
}
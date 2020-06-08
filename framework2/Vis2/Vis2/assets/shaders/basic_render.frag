#version 450 core

vec3 sampleOffset[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

in VertexData {
	vec3 position_model;	
} vert;

out vec4 color;

uniform samplerCube tex;


void main() {
	
	float p = 1 / 10;

	
	int numSamples = 3;
	for (int i = 0; i < numSamples; i++)
	{
		color += texture(tex, vert.position_model + sampleOffset[i] * 0.01);
	}
	
	color /= numSamples;
	color = vec4(0.1,0.1,0.1,1.0) + vec4(color.xyz, 0.0);
}
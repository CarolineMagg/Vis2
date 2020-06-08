#version 450 core

vec3 sampleOffset[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

in vec3 position_model;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;


out vec4 color;

uniform samplerCube tex;
uniform vec3 lightColor;


void main() {
	
	float p = 1 / 10;

	
	int numSamples = 3;
	for (int i = 0; i < numSamples; i++)
	{
		color += texture(tex, position_model + sampleOffset[i] * 0.01);
	}
	
	color /= numSamples;
	if (length(color) < 0.001) {
		color = vec4(0.0,0.0,0.0,1);		
	}
}
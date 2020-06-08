#version 450 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT
{
	vec2 aTexCoords;
} gs_in[];


out vec2 TexCoords;

//out int gl_Layer ;

uniform int glLayer;
uniform mat4 inverseViewMatrix;
uniform float currentZVS;
uniform float sphereRadius;

void main()
{	
	for(int i=0; i<gl_in.length(); i++)
	{
		TexCoords = gs_in[i].aTexCoords;
		gl_Position = gl_in[i].gl_Position;
		gl_Layer = glLayer;			
		EmitVertex();
	}
	EndPrimitive();
}
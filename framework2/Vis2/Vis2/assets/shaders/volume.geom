#version 450 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT
{
	vec2 aTexCoords;
} gs_in[];


out vec2 TexCoords;
out vec3 WorldPosG;

uniform int glLayer;
uniform mat4 inverseViewMatrix;
uniform float currentZVS;

void main()
{	
	for(int i=0; i<3; i++)
	{
		TexCoords = gs_in[i].aTexCoords;
		gl_Position = gl_in[i].gl_Position;
		gl_Layer = glLayer;	
		WorldPosG = vec3((inverseViewMatrix * vec4(gl_in[i].gl_Position.xy * 0.5,0,1)).xy, currentZVS);
		EmitVertex();
	}
	EndPrimitive();
}


//final gs:
////gl_Position = middleOfPlaneVS + vec3(gl_in[i].gl_Position/2.0, planeZOffset);
#version 450 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT
{
	vec2 aTexCoords;
} gs_in[];


out vec2 TexCoords;

uniform mat4 inverseViewMatrix;
uniform mat4 proj;
uniform int glLayer;
uniform vec3 middleOfPlaneVS;
uniform float sphereRadius;

void main()
{	
	for(int i=0; i<3; i++)
	{
		TexCoords = gs_in[i].aTexCoords;
		gl_Position = proj * vec4(middleOfPlaneVS + vec3(gl_in[i].gl_Position.xy/2.0, 0),1);		
		EmitVertex();
	}
	EndPrimitive();
}


//final gs:
////
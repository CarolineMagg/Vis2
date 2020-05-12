#version 450 core
in vec2 TexCoords;
in vec3 WorldPosG;

uniform mat4 inverseViewMatrix;
uniform float planeDistance;
uniform float currentZVS;
uniform vec3 middleOfPlaneVS;

uniform sampler2DArray vpb;
uniform sampler2DArray vdb;
uniform sampler2DArray lb;
uniform sampler2DArray ldb;
uniform sampler2DArray cb;
uniform sampler2DArray mb;

uniform sampler3D volTexture;
uniform sampler2D colorTransfer;

layout (location = 0) out vec4 vpbOut;
layout (location = 1) out vec4 vdbOut;
layout (location = 2) out vec4 lbOut;
layout (location = 3) out vec4 ldbOut;
layout (location = 4) out vec4 cbOut;
layout (location = 5) out vec4 mbOut;


void sampleCentralDifferenceValues(vec3 samplePosition, float sampleDistance, out vec3 s1 , out vec3 s2)
{	
	s1.x = vec4(texture(volTexture, samplePosition - vec3(sampleDistance,0,0))).r; 
	s2.x = vec4(texture(volTexture, samplePosition + vec3(sampleDistance,0,0))).r; 
	s1.y = vec4(texture(volTexture, samplePosition - vec3(0,sampleDistance,0))).r; 
	s2.y = vec4(texture(volTexture, samplePosition + vec3(0,sampleDistance,0))).r;
	s1.z = vec4(texture(volTexture, samplePosition - vec3(0,0,sampleDistance))).r;
	s2.z = vec4(texture(volTexture, samplePosition + vec3(0,0,sampleDistance))).r;	
}

float RefractionTransfer(float value)
{
	// for now just some linear interpolation bteween 1 and 1.45
	return (value * (1.45 - 1.0)) / 255 + 1.0;
}

vec3 RefractionTransfer(vec3 value)
{
	// for now just some linear interpolation bteween 1 and 1.45
	return (value * (1.45 - 1.0)) / 255 + 1.0;
}

vec3 getRefractionGradient(vec3 position)
{
	vec3 s1;
	vec3 s2;
	
	sampleCentralDifferenceValues(position, planeDistance, s1, s2);
	s1 = RefractionTransfer(s1);
	s2 = RefractionTransfer(s2);
	vec3 diff = s2 - s1;

	return length(diff) > 0 ? normalize(s2-s1) : vec3(0);
}




void main() {
	int readLayer = 1 - gl_Layer;
	int writeLayer = gl_Layer;


	// PRE
	//vec4 X = texture(vpb, vec3(TexCoords, readLayer));
	// LIGHT
	vec4 ldi = texture(ldb, vec3(TexCoords, readLayer));
	vec2 lpi_1 = TexCoords - vec2(ldi * planeDistance);


	//TODO worldpos way or this way??
	vec3 volumePosLPI_1 = (inverseViewMatrix * vec4(middleOfPlaneVS.xy + lpi_1 - vec2(0.5), currentZVS, 1.0)).xyz + vec3(0.5);
	vec3 WorldPos = (inverseViewMatrix * vec4(middleOfPlaneVS.xy + TexCoords - vec2(0.5), currentZVS + planeDistance, 1.0)).xyz + vec3(0.5);

	// TODO FILTERING
	vec4 Li_1 = texture(lb, vec3(lpi_1, readLayer));
	vec4 ldi_1 = texture(ldb, vec3(lpi_1, readLayer));

	float Si = abs(dFdx(TexCoords.x)) * abs(dFdy(TexCoords.y));  // DOT CORRECT?
	float Si_1 = abs(dFdx(lpi_1.x)) *  abs(dFdy(lpi_1.y));
	float Ii = Si_1/Si;

	// INTEGRATION TABLE LIGHT
	float volumeX = texture(volTexture, WorldPos).x;
	float volumeLPI_1 = texture(volTexture, volumePosLPI_1).x;
	float alphaL = (volumeX + volumeLPI_1) / 2.0;
	vec3 mL = texture(colorTransfer, vec2(volumeX, volumeLPI_1)).xyz;
	vec4 Li = Li_1 * Ii *  abs(1.0 - alphaL) * (vec4(1.0) - vec4(mL,1.0));

	precise vec3 ref = getRefractionGradient(WorldPos + vec3(0.5)) * planeDistance;
	

	ldi = normalize(ldi_1 + (vec4(ref, 0.0)));

	
	lbOut = Li;
	ldbOut = ldi;

		cbOut = vec4(abs(Li).xyz, 1);   //vec4(abs(ref.xyz),1);//vec4(mL, alphaL, Ii, 1.0);//abs(Li);		
		

	// VIEW

	vec4 vpi = texture(vpb, vec3(TexCoords, readLayer));
	vec4 vdi = texture(vdb, vec3(TexCoords, readLayer));
	vec4 c = texture(cb, vec3(TexCoords, readLayer));
	vec3 Ci_1 = c.xyz;
	float Ai_1 = c.w;
	vec4 Mi_1 = texture(mb, vec3(TexCoords, readLayer));
	vec4 id = texture(lb, vec3(TexCoords, readLayer));



	
	vec4 modelColor = texture(volTexture, vec3(inverseViewMatrix * vpi) + vec3(0.5, 0.5, 0.5));	
	vec4 previousM = texture(mb, vec3(TexCoords, readLayer));		
	mbOut = previousM + vec4(modelColor.r * 0.01,0,0,0);
	
	
	vpbOut = vpi + vdi * planeDistance;
	vdbOut = normalize(vdi +vec4(0,0, -planeDistance,0));
		
	//cbOut  =  vec4(texture(volTexture, WorldPos + vec3(0.5)).xyz,0.5) + vec4(texture(volTexture, WorldPos + vec3(0.5)).yxz,0.5);
	//cbOut.xyz += modelColor.yxz;

	//mbOut = texture(volTexture, vec3((inverseViewMatrix * posT).xy + vec2(0.5, 0.5),0.5));
	//lbOut = modelColor;
	//ldbOut = vec4(vec3(inverseViewMatrix * vpi) + 0.5,1.0);


		
}
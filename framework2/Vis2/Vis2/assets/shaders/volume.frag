#version 450 core
in vec2 TexCoords;
in vec3 WorldPosG;

uniform mat4 inverseViewMatrix;
uniform mat4 viewMatrix;
uniform float planeDistance;
uniform float currentZVS;
uniform vec3 middleOfPlaneVS;
uniform float sphereRadius;

uniform sampler2DArray vpb;
uniform sampler2DArray vdb;
uniform sampler2DArray lb;
uniform sampler2DArray ldb;
uniform sampler2DArray cb;
uniform sampler2DArray mb;
uniform sampler2DArray debug;

uniform sampler3D volTexture;
uniform sampler2D colorTransfer;

layout (location = 0) out vec4 vpbOut;
layout (location = 1) out vec4 vdbOut;
layout (location = 2) out vec4 lbOut;
layout (location = 3) out vec4 ldbOut;
layout (location = 4) out vec4 cbOut;
layout (location = 5) out vec4 mbOut;
layout (location = 6) out vec4 debugOut;


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
	diff = mat3(viewMatrix) * diff;

	return length(diff) > 0 ? normalize(diff) : vec3(0);
}




void main() {
	int readLayer = 1 - gl_Layer;
	int writeLayer = gl_Layer;


	// PRE
	//vec4 X = texture(vpb, vec3(TexCoords, readLayer));
	// LIGHT
	vec4 ldi = texture(ldb, vec3(TexCoords, readLayer));
	vec2 lpi_1 = TexCoords - vec2(ldi * planeDistance);


	//TODO worldpos geometry shader way or this way??
	vec3 volumePosLPI_1 = (inverseViewMatrix * vec4(middleOfPlaneVS.xy + lpi_1 - vec2(0.5), currentZVS, 1.0)).xyz + vec3(0.5);
	vec3 WorldPos = (inverseViewMatrix * vec4(middleOfPlaneVS.xy + TexCoords - vec2(0.5), currentZVS + planeDistance, 1.0)).xyz + vec3(0.5);

	// TODO FILTERING
	vec4 Li_1 = texture(lb, vec3(lpi_1, readLayer));
	vec4 ldi_1 = texture(ldb, vec3(lpi_1, readLayer));

	float Si = abs(dFdx(TexCoords.x)) * abs(dFdy(TexCoords.y));  // DOT CORRECT?
	float Si_1 = abs(dFdx(lpi_1.x)) *  abs(dFdy(lpi_1.y));
	float Ii = 1.0;//Si_1/Si;

	// INTEGRATION TABLE LIGHT
	float volumeX = texture(volTexture, WorldPos).x;
	float volumeLPI_1 = texture(volTexture, volumePosLPI_1).x;
	vec4 transfer = texture(colorTransfer, vec2(volumeX, volumeLPI_1));
	float alphaL = transfer.w / 20.0;
	vec3 mL = 1.0 - transfer.xyz * 0.05;
	vec4 Li = Li_1 * Ii *  abs(1.0 - alphaL) * vec4(mL, 0.0);

	precise vec3 ref = getRefractionGradient(WorldPos) * planeDistance;
	

	ldi = normalize(ldi_1 + (vec4(ref, 0.0)));
	
	lbOut = Li;
	ldbOut = ldi;
			

	// VIEW

	vec4 vpi = texture(vpb, vec3(TexCoords, readLayer));
	vec4 vpiWorldPos = inverseViewMatrix * vpi;
	vec4 vdi = texture(vdb, vec3(TexCoords, readLayer));
	vec4 c = texture(cb, vec3(TexCoords, readLayer));
	vec3 Ci_1 = c.xyz;
	float Ai_1 = c.w;
	vec4 Mi_1 = texture(mb, vec3(TexCoords, readLayer));
	vec4 id = texture(lb, vec3(TexCoords, readLayer));

	// INTEGRATION TABLE VIEW
	vec4 vpi_1 = vpi - vdi * planeDistance;
	float volumeVPI =  texture(volTexture, vec3(vpiWorldPos) + vec3(0.5, 0.5, 0.5)).x;;		
	float volumeVPI_1 =  texture(volTexture, vec3(inverseViewMatrix * vpi_1) + vec3(0.5, 0.5, 0.5)).x;
	vec4 transferV =  texture(colorTransfer, vec2(volumeX, volumeLPI_1)) ;
	vec3 cV = transferV.xyz;
	float alphaV = transferV.w;
	vec3 mV = vec3(1.0) - cV * 0.1;

	vec3 Ci = Ci_1  + ( 1 - min (1.0, Ai_1))  * Mi_1.xyz * ( alphaV *  cV  * id.xyz);
	float Ai = Ai_1  + (1- min(1.0,Ai_1)) * alphaV;
	vec3 Mi = Mi_1.xyz * mV;

	vec4 vdi_P1 = normalize(vdi + planeDistance * vec4(getRefractionGradient(vpiWorldPos.xyz), 0.0));
	vec4 vpi_P1 = vpi + vdi * planeDistance;

	vpbOut = vpi_P1;
	vdbOut = vdi_P1;
	cbOut = vec4(Ci, Ai);
	mbOut = vec4(Mi, 0.0);

	debugOut = vec4(Si_1/Si, Si/Si_1, 0,1);   //vec4(abs(ref.xyz),1);//vec4(mL, alphaL, Ii, 1.0);//abs(Li);	
	
	
}
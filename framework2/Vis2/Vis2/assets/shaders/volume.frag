#version 450 core
in vec2 TexCoords;
in vec3 WorldPosG;

uniform mat4 inverseViewMatrix;
uniform mat4 viewMatrix;
uniform float planeDistance;
uniform float currentZVS;
uniform vec3 middleOfPlaneVS;
uniform float sphereRadius;

uniform float shininess;
uniform float specularCoeff;

uniform sampler2DArray vpb;
uniform sampler2DArray vdb;
uniform sampler2DArray lb;
uniform sampler2DArray ldb;
uniform sampler2DArray cb;
uniform sampler2DArray mb;
uniform sampler2DArray debug;

uniform sampler3D volTexture;
uniform sampler2D volumeTransfer;
uniform sampler2D mediumTransfer;

uniform bool useIi;

layout (location = 0) out vec4 vpbOut;
layout (location = 1) out vec4 vdbOut;
layout (location = 2) out vec4 lbOut;
layout (location = 3) out vec4 ldbOut;
layout (location = 4) out vec4 cbOut;
layout (location = 5) out vec4 mbOut;
layout (location = 6) out vec4 debugOut;

void sampleCentralDifferenceValues(vec3 samplePosition, float sampleDistance, out vec3 s1 , out vec3 s2)
{	
	s1.x = texture(volTexture, samplePosition - vec3(sampleDistance,0,0)).r; 
	s2.x = texture(volTexture, samplePosition + vec3(sampleDistance,0,0)).r; 
	s1.y = texture(volTexture, samplePosition - vec3(0,sampleDistance,0)).r; 
	s2.y = texture(volTexture, samplePosition + vec3(0,sampleDistance,0)).r;
	s1.z = texture(volTexture, samplePosition + vec3(0,0,sampleDistance)).r;
	s2.z = texture(volTexture, samplePosition - vec3(0,0,sampleDistance)).r;	
}

float RefractionTransfer(float value)
{
	if (value < 0.2)
		return 2.0;
	if (value < 0.5)
		return (value * (1.45 - 1.0)) + 1.0;
	if (value < 0.7)
		return (value * (1.65 - 1.45)) + 1.45;
	
	return (value * (2.5 - 1.5)) + 1.5;	
}

vec3 RefractionTransfer(vec3 value)
{		
	return vec3(RefractionTransfer(value.x), RefractionTransfer(value.y), RefractionTransfer(value.z));
}

vec3 getRefractionGradient(vec3 position)
{
	vec3 s1;
	vec3 s2;
	
	sampleCentralDifferenceValues(position, planeDistance, s1, s2);
	s1 = RefractionTransfer(s1);
	s2 = RefractionTransfer(s2);
	//vec3 diff = (s1 + s2) / 2.0;
	vec3 diff = (s2 - s1) ;
	diff = mat3(viewMatrix) * diff;

	return length(diff) > 0 ? normalize(diff) : vec3(0);
}

vec3 getVolumeGradient(vec3 position)
{
	vec3 s1;
	vec3 s2;
	
	sampleCentralDifferenceValues(position, planeDistance, s1, s2);
	vec3 diff = (s2 - s1) ;
	diff = mat3(viewMatrix) * diff;

	return length(diff) > 0 ? normalize(diff) : vec3(0);
}

// assume normalized ray/n, no parallelity
vec3 intersectPlane(vec3 n, vec3 p0, vec3 r, vec3 r0)
{
	float d = dot(n, r);
	//if (abs(d) < 1e-6) return r0;
	//float t = dot((p0-r0), n) / d;
	float t = -(dot(r0, n) + p0.z) / d;
	return r0 + r * t;
}


void main() {
	int readLayer = 1 - gl_Layer;
	int writeLayer = gl_Layer;

	// LIGHT
	vec4 ldi = texture(ldb, vec3(TexCoords, readLayer));

	
	vec2 lpi_1 = intersectPlane(
		vec3(0,0,-1),
		vec3(0.5, 0.5, currentZVS + planeDistance),
		-ldi.xyz,
		vec3(TexCoords, currentZVS)
	).xy;

	//TODO worldpos geometry shader way or this way??
	vec3 volumePosLPI_1 = (inverseViewMatrix * vec4(middleOfPlaneVS.xy + lpi_1 - vec2(0.5), currentZVS + planeDistance, 1.0)).xyz + vec3(0.5);
	vec3 WorldPos = (inverseViewMatrix * vec4(middleOfPlaneVS.xy + TexCoords - vec2(0.5), currentZVS, 1.0)).xyz + vec3(0.5);

	// TODO FILTERING
	vec4 Li_1 = texture(lb, vec3(lpi_1, readLayer));
	Li_1 += texture(lb, vec3(lpi_1 + vec2(1)/512.0, readLayer)) * 0.05;
	Li_1 += texture(lb, vec3(lpi_1 + vec2(-1)/512.0, readLayer)) * 0.05;
	Li_1 += texture(lb, vec3(lpi_1 + vec2(1, -1)/512.0, readLayer)) * 0.05;
	Li_1 += texture(lb, vec3(lpi_1 + vec2(-1, 1)/512.0, readLayer)) * 0.05;
	Li_1 = Li_1 / 1.20;
	
	
	vec4 ldi_1 = texture(ldb, vec3(lpi_1, readLayer));
	ldi_1 += texture(ldb, vec3(lpi_1 + vec2(1,0)/512.0, readLayer)) * 0.25;
	ldi_1 += texture(ldb, vec3(lpi_1 + vec2(-1,0)/512.0, readLayer))* 0.25;
	ldi_1 += texture(ldb, vec3(lpi_1 + vec2(0, -1)/512.0, readLayer))* 0.25;
	ldi_1 += texture(ldb, vec3(lpi_1 + vec2(0, 1)/512.0, readLayer))* 0.25;
	ldi_1 = normalize(ldi_1 / 2.0);

	float Si = abs(dFdxFine(TexCoords.x)) * abs(dFdyFine(TexCoords.y));  // DOT CORRECT?
	float Si_1 = abs(dFdxFine(lpi_1.x)) *  abs(dFdyFine(lpi_1.y));
	
	float Ii = Si_1/Si; //IC

	if (!useIi) Ii = 1.0;

	// INTEGRATION TABLE LIGHT
	float volumeX = texture(volTexture, WorldPos).x;
	float volumeLPI_1 = texture(volTexture, volumePosLPI_1).x;
	vec4 transfer = texture(volumeTransfer, vec2(volumeX, volumeLPI_1));
	float alphaL = transfer.w;
	vec3 mL = texture(mediumTransfer, vec2(volumeX, volumeLPI_1)).xyz;//1.0 - transfer.xyz * 0.05; // transfer medium lpi_1
	vec4 Li = Li_1 * Ii *  abs(1.0 - alphaL);// * vec4(mL, 0.0);

	precise vec3 ref = getRefractionGradient((WorldPos+volumePosLPI_1)/2.0) * planeDistance;
	ldi = normalize(ldi_1 + (vec4(ref, 0.0)));
	
	lbOut = vec4(Li.xyz, 1.0);
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
	vec3 halfwayDir = normalize(ldi + vdi).xyz;
	vec3 normal = getVolumeGradient((WorldPos+volumePosLPI_1)/2.0);	
	vec3 is = Li.xyz * specularCoeff * pow(max(dot(halfwayDir, normal), 0.0),shininess);//vec4(0.0); //TODO: specular component

	// INTEGRATION TABLE VIEW
	//vec4 vpi_1 = vpi - vdi * planeDistance;
	vec4 vpi_1 = vec4(intersectPlane(
		vec3(0,0,-1),
		vec3(0, 0, currentZVS + planeDistance),
		-vdi.xyz,
		vpi.xyz
	), 1.0);

	vec4 vpi_1WorldPos = inverseViewMatrix * vpi_1;
	float volumeVPI =  texture(volTexture, vec3(vpiWorldPos) + vec3(0.5, 0.5, 0.5)).x;;		
	float volumeVPI_1 =  texture(volTexture, vec3(vpi_1WorldPos) + vec3(0.5, 0.5, 0.5)).x;
	vec4 transferV =  texture(volumeTransfer, vec2(volumeX, volumeLPI_1)) ;
	vec3 cV = transferV.xyz;
	float alphaV = transferV.w;
	vec3 mV = texture(mediumTransfer, vec2(volumeX, volumeLPI_1)).xyz;//vec3(1.0) - cV * 0.1; // transfer medium vpi_1

	vec3 Ci = Ci_1  + (1.0 - Ai_1)  /** Mi_1.xyz*/ * ( alphaV *  cV  * id.xyz + is.xyz ); //TODO: add + is.xyz
	float Ai = Ai_1  + (1.0 - Ai_1) * alphaV;
	vec3 Mi = Mi_1.xyz * mV;

	vec4 vdi_P1 = normalize(vdi + planeDistance * vec4(getRefractionGradient((vpiWorldPos.xyz + vpi_1WorldPos.xyz)/2.0 + vec3(0.5, 0.5, 0.5)), 0.0));
	
	//vec4 vpi_P1 = vpi + vdi_P1 * planeDistance;
	vec4 vpi_P1 = vec4(intersectPlane(
		vec3(0,0,-1),
		vec3(0, 0, currentZVS - planeDistance),
		vdi_P1.xyz,
		vpi.xyz
	), 1.0);

	vpbOut = vpi_P1;
	vdbOut = vdi_P1;
	cbOut = vec4(Ci, Ai);
	mbOut = vec4(Mi, 0.0);


	debugOut = vec4(Ci.xyz, 1.0);   //vec4(abs(ref.xyz),1);//vec4(mL, alphaL, Ii, 1.0);//abs(Li);		
		
}
#version 460 core
#extension GL_ARB_bindless_texture : enable
out vec4 FragColor;

in vec2 TexCoords;

layout(bindless_sampler) uniform sampler3D perlworl;		// 3d texture with the low frequency perlin-worley noise. Used to model the base shape of the clouds
layout(bindless_sampler) uniform sampler3D worl;			// 3d texture with the high frequency worley noise. Used to carve out details from the clouds base shape

uniform vec3 camPos;
uniform float time;
uniform vec3 lightDir;
layout(bindless_sampler) uniform sampler2D uNoise;

layout(std140, binding = 1) uniform MeshData
{
    mat4 view;
    mat4 projection;
};

uniform mat4 invView;	// inverse of the view matrix
uniform mat4 invProj;	// inverse of the projection matrix

layout(std430, binding = 11) buffer CloudSSBO {
	// cloud density uniforms
	float coverageScale;							// - the more coverage, the more clouds will appear and vice versa					
	float cloudType;								// - determines which type of clouds will appear, including transitions. 0=stratus, 0.5=stratocumulus, 1=cumulus
	float lowFrequencyNoiseScale;					// - scale factor for the uv-texture coordinates for the low frequency noise texture lookup. Lower values result in 
	int ignoreDetailNoise;							// - set this to true to not use any high frequency noise for cloud modeling, resulting in a great loss of detail (more cartoony look!)

	float highFrequencyNoiseScale;					// - same principle as with the lowFrequencyNoiseScale, but for high frequency noise. Lower means fewer but bigger details, Higher means more but smaller details
	float highFrequencyNoiseErodeMuliplier;			// - used to erode the cloud details away. More means more cuts on the clouds
	float highFrequencyHeightTransitionMultiplier;	// - used to create billowy cloud shapes over height
	float anvilBias;								// - bias for the creation of anvil cloud shapes. Only takes effect with very high clouds (e.g. cumulus!) by inflating density at high heights

	// cloud lighting
	vec3 cloudColor;								// - base color of the clouds (full white). Can be used to create purple/red clouds, etc.
	float padding1;

	float sunIntensity;								// - exponential factor for the sun color. The higher, the more higher is the sun influence on the clouds 
	float ambientColorScale;						// - scale factor with which the ambient cloud color is being multiplied. If increased, the clouds will be lighter
	float rainCloudAbsorptionGain;					// - light absorption factor for the clouds. Used to increase the light absorption of the clouds with beers law. The higher, the darker the clouds
	float cloudAttenuationScale;					// - additional cloud attenuation factor, which is used as a multiplier to the result of the beers law. The higher, the more lightEnergy will be preserved

	float phaseEccentricity;						// - used for the henyey greenstein phase function. Determines the directional influence of the sun lighting, resulting in a silver lining effect
	float phaseSilverLiningIntensity;				// - increase this to strengthen the silver lining effect intensity
	float phaseSilverLiningSpread;					// - is used to influence the spread of the silver lining effect
	float coneSpreadMultplier;						// - length of the cone which is used to gather lighting samples in the cloud volume. The lower, the more influence nearby clouds will have on the lighting samples

	float shadowSampleConeSpreadMultiplier;			// - same as above, but higher (3x higher for example). Used to get a lighting sample from far away clouds
	float powderedSugarEffectMultiplier;			// - used to influence the powdered sugar effect, which occurs when looking at the clouds from the same direction as the light source. Decrease this to see diminish the effect
	float toneMapperEyeExposure;					// - eye exposure for the reinhard tonemapping. The higher, the more brighterthe whole scene becomes
	float maxRenderDistance;						// - this is the max distance where clouds are raymrched (see raySphereCollision check!). IMPORTANT: Decrease this inside the cloud volume (e.g. 70000) to reduce banding artifacts!

	float maxHorizontalSampleCount;					// - raymarch sample count when looking to the horizon. Needs to be higher than the vertical sample count, or far away clouds become less detailed
	float maxVerticalSampleCount;					// - raymarch sample count when looking straight up. Can be lower then the above sample count, since the raymarch is shorter when looking up
	int useEarlyExitAtFullOpacity;					// - used to exit the raymarch when density is at 99%, which greatly increases performance. Disable this to see the performance loss!
	int useBayerFilter;							// - used to activate the offsetting of the raymarch start positions with the below bayer matrix. This reduces banding artifacts

	float earthRadius;								// - radius of the earth. At this height, the camera is initialized. This technaiccaly models the game world ground terrain 
	float volumetricCloudsStartRadius;				// - height at which the volumetric clouds start
	float volumetricCloudsEndRadius;				// - height at which the volumetric clouds end
	float padding2;

	// windsettings
	vec3 windDirection;			// determines the direction in which the clouds will be moved
	float padding3;

	float windUpwardBias;		// bias value which is used to animate the clouds upward over time in order to simulate the shearing effect of rising clouds due to heat
	float cloudSpeed;			// speed at which the clouds will move
	float cloudTopOffset;		// cloud top offset pushes the tops of the clouds along the wind direction by this many units
	float padding4;

	vec2 resolution;
	vec2 padding5;
};


// Bayer filter for adding random offsets to the raymarch start positions to reduce banding artifacts at lower sample counts 
#define BAYER_FACTOR 1.0/16.0
uniform float bayerFilter[16u] = float[]
(
	0.0 * BAYER_FACTOR, 8.0 * BAYER_FACTOR, 2.0 * BAYER_FACTOR, 10.0 * BAYER_FACTOR,
	12.0 * BAYER_FACTOR, 4.0 * BAYER_FACTOR, 14.0 * BAYER_FACTOR, 6.0 * BAYER_FACTOR,
	3.0 * BAYER_FACTOR, 11.0 * BAYER_FACTOR, 1.0 * BAYER_FACTOR, 9.0 * BAYER_FACTOR,
	15.0 * BAYER_FACTOR, 7.0 * BAYER_FACTOR, 13.0 * BAYER_FACTOR, 5.0 * BAYER_FACTOR
	);

// debug variables
uniform bool debugBool = false;
uniform float debugFloat = 1.0;
uniform float debugFloat2 = 0.0;
uniform float debugFloat3 = 1.0;

// preetham model input from vertex shader
in vec3 vSunDirection;
in float vSunfade;
in vec3 vBetaR;
in vec3 vBetaM;
in float vSunE;


/*
================================================================================================================================================================
	Utility Functions
================================================================================================================================================================
*/


// function that maps a value from one range to another
float remap(const float originalValue, const float originalMin, const float originalMax, const float newMin, const float newMax)
{
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

// Henyey-Greenstein phase function
float HG(float costheta, float g) {
	const float k = 0.0795774715459;
	return k * (1.0 - g * g) / (pow(1.0 + g * g - 2.0 * g * costheta, 1.5));
}


/*
================================================================================================================================================================
	begin of preetham atmospheric scattering model
================================================================================================================================================================
*/

/**
 * Based on "A Practical Analytic Model for Daylight"
 * aka The Preetham Model, the de facto standard analytic skydome model
 * https://www.researchgate.net/publication/220720443_A_Practical_Analytic_Model_for_Daylight
 *
 * First implemented by Simon Wallner
 * http://simonwallner.at/project/atmospheric-scattering/
 *
 * Improved by Martin Upitis
 * http://blenderartists.org/forum/showthread.php?245954-preethams-sky-impementation-HDR
 *
 * Three.js integration by zz85 (http://twitter.com/blurspline) here: https://github.com/mrdoob/three.js/blob/master/examples/js/objects/Sky.js
 *
 * The implementation used here is based on the Three.js integration
*/

const float mieDirectionalG = 0.8;

// constants for atmospheric scattering
const float pi = 3.141592653589793238462643383279502884197169;

// optical length at zenith for molecules
const float rayleighZenithLength = 8.4E3;
const float mieZenithLength = 1.25E3;
const vec3 up = vec3(0.0, 1.0, 0.0);
// 66 arc seconds -> degrees, and the cosine of that
const float sunAngularDiameterCos = 0.999956676946448443553574619906976478926848692873900859324;

// 3.0 / ( 16.0 * pi )
const float THREE_OVER_SIXTEENPI = 0.05968310365946075;

vec3 getColorFromPreethamAtmosphere(vec3 directionToAtmosphere)
{
	// optical length
	// cutoff angle at 90 to avoid singularity in next formula.
	float zenithAngle = acos(max(0.0, dot(up, normalize(directionToAtmosphere))));
	float inv = 1.0 / (cos(zenithAngle) + 0.15 * pow(93.885 - ((zenithAngle * 180.0) / pi), -1.253));
	float sR = rayleighZenithLength * inv;
	float sM = mieZenithLength * inv;

	// combined extinction factor
	vec3 Fex = exp(-(vBetaR * sR + vBetaM * sM));

	// in scattering
	float cosTheta = dot(normalize(directionToAtmosphere), vSunDirection);

	float rPhase = THREE_OVER_SIXTEENPI * (1.0 + pow(cosTheta * 0.5 + 0.5, 2.0));
	vec3 betaRTheta = vBetaR * rPhase;

	float mPhase = HG(cosTheta, mieDirectionalG);
	vec3 betaMTheta = vBetaM * mPhase;

	vec3 Lin = pow(vSunE * ((betaRTheta + betaMTheta) / (vBetaR + vBetaM)) * (1.0 - Fex), vec3(1.5));
	Lin *= mix(vec3(1.0), pow(vSunE * ((betaRTheta + betaMTheta) / (vBetaR + vBetaM)) * Fex, vec3(1.0 / 2.0)), clamp(pow(1.0 - dot(up, vSunDirection), 5.0), 0.0, 1.0));

	vec3 L0 = vec3(0.5) * Fex;

	// composition + solar disc
	float sundisk = smoothstep(sunAngularDiameterCos, sunAngularDiameterCos + 0.00002, cosTheta);
	L0 += (vSunE * 19000.0 * Fex) * sundisk;

	vec3 texColor = (Lin + L0) * 0.04 + vec3(0.0, 0.0003, 0.00075);

	vec3 retColor = pow(texColor, vec3(1.0 / (1.2 + (1.2 * vSunfade))));

	return retColor;
}


/*
================================================================================================================================================================
	begin of volumetric clouds
================================================================================================================================================================
*/

// random vectors for cone lighting sampling
const vec3 RANDOM_VECTORS[6] = vec3[6]
(
	vec3(0.38051305f, 0.92453449f, -0.02111345f),
	vec3(-0.50625799f, -0.03590792f, -0.86163418f),
	vec3(-0.32509218f, -0.94557439f, 0.01428793f),
	vec3(0.09026238f, -0.27376545f, 0.95755165f),
	vec3(0.28128598f, 0.42443639f, -0.86065785f),
	vec3(-0.16852403f, 0.14748697f, 0.97460106f)
	);

// get global fractional position in the cloud volume, clamped to [0,1]
float GetHeightFractionForPoint(vec3 worldPosition)
{
	// cloud sphere has its origin at (0,0,0). If the origin was not at (0,0,0), an additional vector (worldPosition - sphereOrigin) needs to be calculated here
	float heightFraction = (length(worldPosition /* - volumetricCloudVolumeOrigin */) - volumetricCloudsStartRadius) / (volumetricCloudsEndRadius - volumetricCloudsStartRadius);
	return clamp(heightFraction, 0.0, 1.0);
}

vec4 mixGradients(const float cloudType)
{
	/*
	// Density Height Gradients from Clay John's volumetric cloud project:
	// https://github.com/clayjohn/realtime_clouds/blob/master/assets/shaders/sky.frag
	// These gradients have the property, that the cumulus clouds (cloudType = 1.0) grow upward AND downward due to the fact, that during the transition
	// from stratocumulus to cumulus, the cloud density increases at lower heights.
	const vec4 STRATUS_GRADIENT = vec4(0.02f, 0.05f, 0.09f, 0.11f);
	const vec4 STRATOCUMULUS_GRADIENT = vec4(0.02f, 0.2f, 0.48f, 0.625f);
	const vec4 CUMULUS_GRADIENT = vec4(0.01f, 0.0625f, 0.78f, 1.0f);
	*/

	// Density Height Gradients taken from NadirRoGue's volumetric cloud project: 
	// https://github.com/NadirRoGue/RenderEngine/blob/master/RenderEngine/RenderEngine/shaders/clouds/volumetricclouds.frag
	// Since they look and feel very good, we just use these gradients. But the above gradients from Clay John are also worth a consideration!
	const vec4 STRATUS_GRADIENT = vec4(0.0, 0.1, 0.2, 0.3);
	const vec4 STRATOCUMULUS_GRADIENT = vec4(0.02, 0.2, 0.48, 0.625);
	const vec4 CUMULUS_GRADIENT = vec4(0.0, 0.1625, 0.88, 0.98);

	// mix the gradients, so that we get a gradient containing contributions from the three cloud formation types based on the cloudType-variable
	float stratus = 1.0f - clamp(cloudType * 2.0f, 0.0, 1.0);
	float stratocumulus = 1.0f - abs(cloudType - 0.5f) * 2.0f;
	float cumulus = clamp(cloudType - 0.5f, 0.0, 1.0) * 2.0f;
	return STRATUS_GRADIENT * stratus + STRATOCUMULUS_GRADIENT * stratocumulus + CUMULUS_GRADIENT * cumulus;
}

float densityHeightGradient(const float heightFraction, const float cloudType)
{
	vec4 cloudGradient = mixGradients(cloudType);

	// calculate the density height gradient. The below formular was based on the Siggraph 2017 Nubis-Decima talk from Guerilla Games for the Game "Horizon Zero Dawn".
	//float density = remap(heightFraction, cloudGradient.x, cloudGradient.y, 0.0, 1.0) * remap(heightFraction, cloudGradient.z, cloudGradient.w, 1.0, 0.0);

	// A better way to calculate the density height gradient is the formular below, which smoothly transitions the densities based on the height fraction.
	// For help in understanding this formular, visit https://thebookofshaders.com/05/ and read the chapter "Step and Smoothstep". At the second code paragraph, the below formular 
	// can be seen in action. Thanks to thebookofshaders!
	float density = smoothstep(cloudGradient.x, cloudGradient.y, heightFraction) - smoothstep(cloudGradient.z, cloudGradient.w, heightFraction);
	return density;
}

bool raySphereIntersection(const vec3 rayOrigin, const vec3 rayDirection, const float sphereRadius, out vec3 furthestIntersectionPoint, float maxIntersectionDistance, out bool isIntersectionTooFarAway)
{
	float b = 2 * dot(rayDirection, rayOrigin);
	float c = dot(rayOrigin, rayOrigin) - sphereRadius * sphereRadius;
	float discriminant = b * b - (4 * c);
	// no intersection with sphere at all?
	if (discriminant < 0.0f) {
		return false;
	}
	float squareRoot = sqrt(discriminant);
	float t1 = (-b - squareRoot) * 0.5;
	float t2 = (-b + squareRoot) * 0.5;
	// get distance to furthest intersection point along the ray direction
	float tMax = max(t1, t2);
	// if furthest distance is negative, then the furthest intersection point lies behind the ray starting point in the opposite ray direction,
	// which counts as no intersection here
	if (tMax < 0.0) {
		return false;
	}
	// is the distance to the furthest intersection point greater then a supplied threshold? (e.g. greater then the max render distance?) Then the intersection counts
	// as not valid, which can be used as a useful information
	isIntersectionTooFarAway = false;
	if (tMax > maxIntersectionDistance) {
		isIntersectionTooFarAway = true;
	}
	// calculate the furthest intersection point
	furthestIntersectionPoint = rayOrigin + rayDirection * tMax;
	return true;
}




// This function is a modified version of the example code from the book "GPU Pro 7", chapter 4: "Real-Time Volumetric Cloudscapes"
// Parameter "p" is the current world position inside the cloud volume. This position will be continually updated during the raymarch
float getCloudDensity(vec3 p, const bool expensiveSample, const float LOD)
{
	// get height fraction in [0,1] interval for the current position p in the cloud volume.
	// IMPORTANT: The height fraction must be calculated before offsetting the position with the following wind settings. The clouds will otherwise disappear over time!
	float heightFraction = GetHeightFractionForPoint(p);

	// Wind Settings
	//

	// adjust windDirection, so that the y component is inverted in order for the clouds to flow upward and not downward)
	vec3 adjustedWindDirection = vec3(windDirection.x, -windDirection.y, windDirection.z);
	p += heightFraction * adjustedWindDirection * cloudTopOffset;   // skew clouds in wind direction
	p += (adjustedWindDirection + vec3(0.0, -windUpwardBias, 0.0)) * time * cloudSpeed;    // animate clouds in wind direction and add a small upward bias to the wind direction

	// Cloud Sampling
	//

	// sample the low frequency shape noise texture
	vec4 lowFrequencyNoises = textureLod(perlworl, p * 0.00003 * lowFrequencyNoiseScale, LOD);

	// build an FBM out of the low frequency worley noises that will be used to add detail to the low-frequency perlin-worley noise
	float lowFrequencyFBM = lowFrequencyNoises.g * 0.625 + lowFrequencyNoises.b * 0.25 + lowFrequencyNoises.a * 0.125;

	// define the base cloud shape by dilating it with the low frequency FBM made of worley noise
	float baseCloud = remap(lowFrequencyNoises.r, -(1.0 - lowFrequencyFBM), 1.0, 0.0, 1.0);

	// get the density-height gradient based on the cloud type. 0.0=stratus, 0.5=stratocumulus, 1.0=cumulus
	float densityHeightGradient = densityHeightGradient(heightFraction, cloudType);

	// apply the density heightGradient to the base cloud shape
	baseCloud *= densityHeightGradient;

	// use remap to apply the cloud coverage attribute. The more coverage, the more clouds are being let through. The less coverage, the more clouds 
	// are mapped to zero density and therefore not being rendered, resulting in a clearer sky.
	// In the opposite case (originalMin = coverageScale, instead of originalMin = 1 - coverageScale), the coverageScale value represents a 
	// threshold-gate which roughly says: Clouds with a density under that threshold are reduced to below 0 density and therefore not being rendered
	float baseCloudWithCoverage = remap(baseCloud, 1 - coverageScale, 1.0, 0.0, 1.0);

	// multiplay the result by the cloud coverage attribute so that smaller clouds are lighter and more aesthetically pleasing
	baseCloudWithCoverage *= coverageScale;

	// use high-frequency noise?
	if (expensiveSample && ignoreDetailNoise==0) {

		// erode base cloud with curl noise. (NOT USED HERE, since no curl noise is being used in this demo!)
		//vec2 whisp = texture(curl, p.xy*0.0003).rg;
		//p.xy += whisp*400.0*(1.0-height_fraction);	

		// sample high-frequency worley noise (=detail noise)
		vec3 highFrequencyNoises = texture(worl, p * 0.001 * highFrequencyNoiseScale, LOD).rgb;

		// build high frequency worley noise fbm
		float highFrequencyFBM = highFrequencyNoises.r * 0.625 + highFrequencyNoises.g * 0.25 + highFrequencyNoises.b * 0.125;

		// get the height fraction for use with blending noise types over height (NOT USED HERE, since no curl noise is being used in this demo!)
		//heightFraction = GetHeightFractionForPoint(p);

		// transition from wispy shapes to billowy shapes over height
		float highFrequencyNoiseModifier = mix(highFrequencyFBM, 1.0 - highFrequencyFBM, clamp(heightFraction * highFrequencyHeightTransitionMultiplier, 0.0, 1.0));

		// erode the base cloud shape with the distorted high-frequency noise (for fluffy or detail-rich clouds). Think of carving a detailed cloud out of a block of clay
		baseCloudWithCoverage = remap(baseCloudWithCoverage, highFrequencyNoiseModifier * highFrequencyNoiseErodeMuliplier, 1.0, 0.0, 1.0);
	}

	// let high clouds form anvil-like structures based on a bias (0 = no influence), by inflating density at mid to high layers
	baseCloudWithCoverage = pow(baseCloudWithCoverage, remap(heightFraction, 0.6, 0.8, 1.0, mix(1.0, 0.5, anvilBias)));
	return clamp(baseCloudWithCoverage, 0.0, 1.0);
}

// This function is a modified version of the example code from the book "GPU Pro 7", chapter 4: "Real-Time Volumetric Cloudscapes"
vec4 raymarch(const vec3 startPosition, const vec3 endPosition, vec3 raymarchStepVector, const int sampleCount, vec3 sunColor, vec3 ambientLightColor)
{
	// the Position in the Cloud Volume is the raymarch start position
	vec3 positionInCloudVolume = startPosition;

	// dithering on the starting ray position to reduce banding artifacts 
	if (useBayerFilter==1) {
		int a = int(gl_FragCoord.x) % 4;
		int b = int(gl_FragCoord.y) % 4;
		positionInCloudVolume += raymarchStepVector * bayerFilter[a * 4 + b];
	}

	// This is the vector pointing towards the sun. It has the same length as the raymarchStepVector in order to sample the densities in a consistent way
	vec3 lightStepVector = vSunDirection * length(raymarchStepVector);

	// cone spread will be used to sample the surrounding cloud densities for lighting calculations. The higher the cone spread, the less shadowy the clouds become,
	// since more samples will lie outside of the cloud volume 
	float coneSpread = length(lightStepVector) * coneSpreadMultplier;

	// angle between direction to light and raymarch direction
	float costheta = dot(normalize(lightStepVector), normalize(raymarchStepVector));

	// calculate the directional scattering propability, which accounts for the silver lining effect in the clouds
	float phase = max(HG(costheta, phaseEccentricity), phaseSilverLiningIntensity * HG(costheta, 0.99 - phaseSilverLiningSpread));

	// start raymarching
	vec3 resultCloudColor = vec3(0.0);
	float resultCloudOpacity = 0;
	float cloudTest = 0.0;
	int zeroDensitySampleCount = 0;
	for (int i = 0; i < sampleCount; i++)
	{
		// cloud test starts as zero so we always evaluate second case from the beginning
		if (cloudTest > 0.0)
		{
			// sample density the expensive way (parameter "true" indicates expensive sample)
			float sampledCloudDensity = getCloudDensity(positionInCloudVolume, true, 0.0);

			// if we just sampled a zero, increment the counter
			if (sampledCloudDensity == 0.0)
			{
				zeroDensitySampleCount++;
			}

			// if we are doing an expensive sample that is still potentially in the cloud:
			if (zeroDensitySampleCount != 6)
			{
				// if sampledCloudDensity is not zero (meaning we are in a cloud) => calculate lighting
				if (sampledCloudDensity != 0.0)
				{
					vec3 lightSamplePositionInCloudVolume = positionInCloudVolume;
					float sampledDensityAlongLightCone = 0.0;

					// sample cloud density along cone and accumulate density along light ray
					for (int j = 0; j < 6; j++)
					{
						// get random light sample position in a cone
						lightSamplePositionInCloudVolume += (lightStepVector + (coneSpread * RANDOM_VECTORS[j] * float(j)));

						// if the accumulated density along the lighting cone has surpassed a threshold value where its light contribution can be
						// more generalized (here 0.3, same as in Horizon: Zero Dawn), we switch to cheap sampling as an optimization with little visual impact
						if (sampledDensityAlongLightCone < 0.3)
						{
							// sample the lighting densities at consecutive higher mip-levels, so that the banding effects are reduced
							sampledDensityAlongLightCone += getCloudDensity(lightSamplePositionInCloudVolume, true, float(j + 1));	// expensive sample
						}
						else
						{
							sampledDensityAlongLightCone += getCloudDensity(lightSamplePositionInCloudVolume, false, 1);	// cheap sample
						}
					}

					// take additional far step (3x coneSpread based on GPU Pro 7 article) for calculating shadowing from other clouds onto the current sample
					lightSamplePositionInCloudVolume += lightStepVector * coneSpread * shadowSampleConeSpreadMultiplier;
					sampledDensityAlongLightCone += getCloudDensity(lightSamplePositionInCloudVolume, false, 1.0);	// cheap sample

					// calculate light energy
					//

					// beers law: calculate attenuation for lighting sample
					float beers = exp(-sampledDensityAlongLightCone * rainCloudAbsorptionGain) * cloudAttenuationScale;

					// calculate powdered sugar effect, which approximates the inscattering of light when looking to the clouds from the same direction as the light source (sun)  
					float powderSugarEffect = 1.0 - exp(-2.0 * sampledDensityAlongLightCone * powderedSugarEffectMultiplier);

					// finally, calculate the lightEnergy
					float lightEnergy = 2.0 * beers * powderSugarEffect * phase;


					// calculate the cloud color
					//

					// calculate the ambient light color influence (of the "blue-ish" ambient light color retrieved from the preetham model) on the cloud color  
					vec3 ambientLightColorComponent = ambientLightColor * ambientColorScale;

					// calculate the sun color influence (of the "yellow/white-ish" sun color retrieved from the preetham model) on the cloud color. The influence
					// of the sun color on the clouds is best seen at sunsets, where the clouds become bright yellow under the suns influence
					vec3 sunColorComponent = pow(sunColor, vec3(sunIntensity));

					// calculate the current cloud color in this iteration
					vec3 cloudColor = vec3(ambientLightColorComponent + cloudColor * sunColorComponent * lightEnergy);

					// Accumulate the cloud result color and result opacity. Equations are based off https://www.willusher.io/webgl/2019/01/13/volume-rendering-with-webgl
					//

					// Accumulate Color
					resultCloudColor = resultCloudColor + (1.0 - resultCloudOpacity) * cloudColor * sampledCloudDensity;
					// Accumulate Opacity
					resultCloudOpacity = resultCloudOpacity + (1.0 - resultCloudOpacity) * sampledCloudDensity;

					// early exit the raymarch when the opacity is near 100%. This increases performance significantly and it allows for higher sample counts for more
					// detailed clouds (sample count of 256 becomes feasible for example).
					if (resultCloudOpacity >= 0.99 && useEarlyExitAtFullOpacity==1) {
						resultCloudOpacity = 1.0;
						break;
					}

				}

				// if sampled cloud density was 0, take a step forward for the next density sample iteration
				positionInCloudVolume += raymarchStepVector;
			}
			// if zeroDensitySampleCount == 6, then set cloudTest to zero, so that we go back to the cheap sample case
			else
			{
				cloudTest = 0.0;
				zeroDensitySampleCount = 0;
			}
		}
		else
		{
			// sample density the cheap way, only using the low frequency noise (parameter "false" indicates cheap sample)
			cloudTest = getCloudDensity(positionInCloudVolume, false, 0.0);
			if (cloudTest == 0.0)
			{
				positionInCloudVolume += raymarchStepVector;
			}
		}
	}
	return vec4(resultCloudColor, resultCloudOpacity);
}



void main()
{
	// 1. task: construct a camera direction vector which points from the camera to the fragments world space position
//

// convert the screen space fragment coordinates to texture coordinates (range: [0, 1]) in order to be able to convert them to NDC afterwards
	vec2 screenSpaceFragmentCoordsInTextureCoords = gl_FragCoord.xy / resolution;

	// convert texture coordinates to NDC (range: [-1, 1]) and convert the fragment coordinates to a vector in clip space
	vec4 screenSpaceFragmentCoordsInClipSpace = vec4(vec2(2.0 * screenSpaceFragmentCoordsInTextureCoords - 1.0), 1.0, 1.0);

	// convert the clip-space vector with the inverse projection matrix back to view-space
	vec4 screenSpaceFragmentCoordsInViewSpace = invProj * screenSpaceFragmentCoordsInClipSpace;

	// adjust the above view vector so that it points to the fragment position in view space by setting the z-component to -1 (camera front vector points along negative z-axis!)
	screenSpaceFragmentCoordsInViewSpace = vec4(screenSpaceFragmentCoordsInViewSpace.xy, -1.0, 0.0);	// w=0 since the view vector is a direction, not a position

	// convert the view vector to world space with the inverse view matrix and normalize. This yields a direction vector from the camera to the fragment position in world space,
	// which is needed for the following raymarch algorithm
	vec3 fragmentWorldSpacePosition = (invView * screenSpaceFragmentCoordsInViewSpace).xyz;
	vec3 viewDirectionToFragmentPosition = normalize(fragmentWorldSpacePosition);


	// 2. task: determine if raymarch through the cloud layer is possible by doing intersection tests with the cloud volume to determine the raymarch start- and end world positions
	//

	bool canRaymarch = false;
	vec3 raymarchStartPosition;
	vec3 raymarchEndPosition;

	// adjust the camera position, by adding the earthRadius to its y-component, since the earth radius is basically the game worlds ground terrain
	vec3 cameraPosition = vec3(0.0, 0.0, 0.0);
	cameraPosition.x = camPos.x;
	cameraPosition.y = camPos.y + earthRadius;
	cameraPosition.z = camPos.z;
	float cameraDistanceFromOrigin = length(cameraPosition);

	// is camera below the cloud volume?
	if (cameraDistanceFromOrigin < volumetricCloudsStartRadius)
	{
		// since we are under the cloud volume, both spheres will always be intersected by the viewing ray
		bool isInnerIntersectionTooFarAway = false;
		bool isOuterIntersectionTooFarAway = false;
		raySphereIntersection(cameraPosition, viewDirectionToFragmentPosition, volumetricCloudsStartRadius, raymarchStartPosition, maxRenderDistance, isInnerIntersectionTooFarAway);
		raySphereIntersection(cameraPosition, viewDirectionToFragmentPosition, volumetricCloudsEndRadius, raymarchEndPosition, maxRenderDistance, isOuterIntersectionTooFarAway);
		canRaymarch = true;
		// if the intersection with the inner sphere was too far away, we dont render the clouds at this intersection
		if (isInnerIntersectionTooFarAway) {
			canRaymarch = false;
		}
	}
	// is camera inside or above the cloud volume? Meaning, that the camera is above the volume starting radius?
	else if (cameraDistanceFromOrigin >= volumetricCloudsStartRadius)
	{
		// start position for raymarch inside and above the cloud layer is the current camera position in the cloud layer. This is technically a bandaid and not correct when we are
		// above (and not inside) the cloud volume, but it is not so easy to detect the "above cloud layer"-case, since clouds can have different heights inside the volume
		raymarchStartPosition = cameraPosition;

		// is outer sphere being intersected?
		vec3 outerSpherePointForward;
		bool isOuterIntersectionTooFarAway;
		bool isOuterSphereIntersected = raySphereIntersection(cameraPosition, viewDirectionToFragmentPosition, volumetricCloudsEndRadius, outerSpherePointForward, maxRenderDistance, isOuterIntersectionTooFarAway);

		// if the outer sphere is intersected: The raymarch end position is the minimum between the distance to the intersection point and the maxRenderDistance. This is necessary 
		// because the intersection point is the furthest point of the outer cloud volume sphere. When looking down from inside or above the cloud volume, the ray pierces the outer
		// cloud sphere all the way to the bottom through the game world ground terrain, resulting in incorrect cloud visuals. That is why we take the minimum here. These incorrect
		// visuals can be observed by increasing the maxRenderDistance to a very high value.
		// In order to retrieve the best cloud visual results, the maxRenderDistance to the clouds needs to be reduced inside the clouds or the sampleCount needs to be increased inside
		// the clouds, so that the raymarch can take more samples inside the cloud volume.
		if (isOuterSphereIntersected) {
			float t = length(outerSpherePointForward - cameraPosition);
			raymarchEndPosition = cameraPosition + viewDirectionToFragmentPosition * min(t, maxRenderDistance);
			canRaymarch = true;
		}
	}


	// 3. task: if raymarch start- and end positions are successfully determined -> raymarch the cloud volume and 
	//

	vec3 resultColor = vec3(0.0);
	vec4 cloudColor = vec4(0.0);
	if (canRaymarch)
	{
		float raymarchLength = length(raymarchEndPosition - raymarchStartPosition);
		// determine sampleCount for raymarch. Based on the article on GPU Pro 7, page 21, section 4.5.1., there should by (at least) 128 samples when looking at the horizon and 
		// 64 samples when looking up/down
		float sampleCount = mix(maxHorizontalSampleCount, maxVerticalSampleCount, abs(dot(viewDirectionToFragmentPosition, vec3(0.0, 1.0, 0.0))));
		// calculate the step vector for the raymarch. This is the vector that will used to advance through the cloud volume
		vec3 raymarchStepVector = viewDirectionToFragmentPosition * (raymarchLength / sampleCount);

		// get a "blue-ish" ambient light color from the surrounding atmosphere. This color will be used as the ambient cloud color
		vec3 ambientLightColor = getColorFromPreethamAtmosphere(normalize(vec3(0.3, 0.2, 0.0)));
		// get the yellow sun color from the surrounding atmosphere. This color will be used to additionally dye the clouds (best seen at sunsets!)
		vec3 sunColor = getColorFromPreethamAtmosphere(normalize(vSunDirection));

		// finally raymarch the clouds to retrieve the cloud color and cloud opacity 
		vec4 cloudColor = raymarch(raymarchStartPosition, raymarchEndPosition, raymarchStepVector, int(sampleCount), sunColor, ambientLightColor);

		// blend the atmosphere background color with the clouds with the cloud opacity, so that we can see the sky colors between cloud formations
		vec3 background = getColorFromPreethamAtmosphere(viewDirectionToFragmentPosition);

		resultColor = cloudColor.xyz * cloudColor.a;
	}
	else
	{
		// when not raymarch can be done, just take the color from the atmosphere along the view direction
		discard;
	}


	// 4. task: apply tone mapping
	resultColor = vec3(1.0) - exp(-resultColor.xyz * toneMapperEyeExposure);	// Reinhard Tone Mapping with eye exposure variable

    FragColor = vec4(resultColor, cloudColor.a+0.8);
	gl_FragDepth = 0.999;

}

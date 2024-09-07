#version 460 core
#extension GL_ARB_bindless_texture : enable

/*
	Fills a 3D texture with WORLEY noise to be used in the volumetric
	clouds as shape eroder
*/

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba8, bindless_image) uniform image3D outVolTex;

// =====================================================================================
// Code from Sebastien Hillarie 3d noise generator https://github.com/sebh/TileableVolumeNoise
uniform float frequenceMul[6u] = float[](2.0, 8.0, 14.0, 20.0, 26.0, 32.0);

float hash(int n)
{
	return fract(sin(float(n) + 1.951) * 43758.5453123);
}

float noise(vec3 x)
{
	vec3 p = floor(x);
	vec3 f = fract(x);

	f = f * f * (vec3(3.0) - vec3(2.0) * f);
	float n = p.x + p.y * 57.0 + 113.0 * p.z;
	return mix(
		mix(
			mix(hash(int(n + 0.0)), hash(int(n + 1.0)), f.x),
			mix(hash(int(n + 57.0)), hash(int(n + 58.0)), f.x),
			f.y),
		mix(
			mix(hash(int(n + 113.0)), hash(int(n + 114.0)), f.x),
			mix(hash(int(n + 170.0)), hash(int(n + 171.0)), f.x),
			f.y),
		f.z);
}

float cells(vec3 p, float cellCount)
{
	vec3 pCell = p * cellCount;
	float d = 1.0e10;
	for (int xo = -1; xo <= 1; xo++)
	{
		for (int yo = -1; yo <= 1; yo++)
		{
			for (int zo = -1; zo <= 1; zo++)
			{
				vec3 tp = floor(pCell) + vec3(xo, yo, zo);

				tp = pCell - tp - noise(mod(tp, cellCount / 1.0));

				d = min(d, dot(tp, tp));
			}
		}
	}
	d = min(d, 1.0);
	d = max(d, 0.0f);

	return d;
}

// ======================================================================

float worleyNoise3D(vec3 p, float cellCount)
{
	return cells(p, cellCount);
}

vec4 stackable3DNoise(ivec3 pixel)
{
	vec3 coord = vec3(float(pixel.x) / 32.0, float(pixel.y) / 32.0, float(pixel.z) / 32.0);

	// 3 octaves
	float cellCount = 2.0;
	float worleyNoise0 = (1.0f - worleyNoise3D(coord, cellCount * 1.0));
	float worleyNoise1 = (1.0f - worleyNoise3D(coord, cellCount * 2.0));
	float worleyNoise2 = (1.0f - worleyNoise3D(coord, cellCount * 4.0));
	float worleyNoise3 = (1.0f - worleyNoise3D(coord, cellCount * 8.0));
	/*
	#ifdef CLAMPRESULT
		float worleyFBM0 = clamp(worleyNoise0*0.625f + worleyNoise1*0.25f + worleyNoise2*0.125f, 0.0, 1.0);
		float worleyFBM1 = clamp(worleyNoise1*0.625f + worleyNoise2*0.25f + worleyNoise3*0.125f, 0.0, 1.0);
		float worleyFBM2 = clamp(worleyNoise2*0.75f + worleyNoise3*0.25f, 0.0, 1.0);
	#else
		float worleyFBM0 = worleyNoise0*0.625f + worleyNoise1*0.25f + worleyNoise2*0.125f;
		float worleyFBM1 = worleyNoise1*0.625f + worleyNoise2*0.25f + worleyNoise3*0.125f;
		float worleyFBM2 = worleyNoise2*0.75f + worleyNoise3*0.25f;
	#endif
	*/
	// cellCount=4 -> worleyNoise4 is just noise due to sampling frequency=texel freque. So only take into account 2 frequencies for FBM
	float worleyFBM0 = clamp(worleyNoise0 * 0.625f + worleyNoise1 * 0.25f + worleyNoise2 * 0.125f, 0.0, 1.0);
	float worleyFBM1 = clamp(worleyNoise1 * 0.625f + worleyNoise2 * 0.25f + worleyNoise3 * 0.125f, 0.0, 1.0);
	float worleyFBM2 = clamp(worleyNoise2 * 0.75f + worleyNoise3 * 0.25f, 0.0, 1.0);
	return vec4(worleyFBM0, worleyFBM1, worleyFBM2, 1.0);
}

void main()
{
	ivec3 pixel = ivec3(gl_GlobalInvocationID.xyz);

	// r = Worley X octave
	// g = Worley X + 1 octave
	// b = Worley X + 2 octave
	// res 32 * 32 * 32
	imageStore(outVolTex, pixel, stackable3DNoise(pixel));
}
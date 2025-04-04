#version 460 core
#extension GL_ARB_bindless_texture : enable

/*
	Fills a 2D texture with a low octave perlin noise to be used
	in the volumetric clouds as weather map
*/

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba8, bindless_image) uniform image2D outWeatherTex;


// =====================================================================================
// COMMON
float random2D(in vec2 st)
{
	return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// =====================================================================================
// PERLIN NOISE SPECIFIC

uniform float perlinAmplitude = 0.5;
uniform float perlinFrecuency = 0.92;
uniform float perlinScale = 50.0;
uniform int perlinOctaves = 8;

float noiseInterpolation(in vec2 i_coord, in float i_size)
{
	vec2 grid = i_coord * i_size;

	vec2 randomInput = floor(grid);
	vec2 weights = fract(grid);

	float p0 = random2D(randomInput);
	float p1 = random2D(randomInput + vec2(1.0, 0.0));
	float p2 = random2D(randomInput + vec2(0.0, 1.0));
	float p3 = random2D(randomInput + vec2(1.0, 1.0));

	weights = smoothstep(vec2(0.0, 0.0), vec2(1.0, 1.0), weights);

	return p0 +
		(p1 - p0) * (weights.x) +
		(p2 - p0) * (weights.y) * (1.0 - weights.x) +
		(p3 - p1) * (weights.y * weights.x);
}

float perlinNoise(vec2 uv, float sc, float f, float a, int o)
{
	float noiseValue = 0.0;

	float localAplitude = a;
	float localFrecuency = f;

	for (int index = 0; index < o; index++)
	{

		noiseValue += noiseInterpolation(uv, sc * localFrecuency) * localAplitude;

		localAplitude *= 0.5;
		localFrecuency *= 2.0;
	}


	noiseValue = (noiseValue - 0.2) / 0.8;
	//return clamp(noiseValue, 0.0, 1.0);
	return noiseValue;
	//noiseValue = noiseValue / 1.5;
	//return clamp(noiseValue, 0.0, 1.0);
}

// =====================================================================================

const vec2 _pixelOffset[25u] = vec2[](
	vec2(-2.0, 2.0), vec2(-1.0, 2.0), vec2(0.0, 2.0), vec2(1.0, 2.0), vec2(2.0, 2.0),
	vec2(-2.0, 1.0), vec2(-1.0, 1.0), vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(2.0, 1.0),
	vec2(-2.0, 0.0), vec2(-1.0, 0.0), vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(2.0, 0.0),
	vec2(-2.0, -1.0), vec2(-1.0, -1.0), vec2(0.0, -1.0), vec2(1.0, -1.0), vec2(2.0, -1.0),
	vec2(-2.0, -2.0), vec2(-1.0, -2.0), vec2(0.0, -2.0), vec2(1.0, -2.0), vec2(2.0, -2.0));

const float _maskFactor = float(1.0 / 65.0);

const float _weights[25u] = float[](
	1.0 * _maskFactor, 2.0 * _maskFactor, 3.0 * _maskFactor, 2.0 * _maskFactor, 1.0 * _maskFactor,
	2.0 * _maskFactor, 3.0 * _maskFactor, 4.0 * _maskFactor, 3.0 * _maskFactor, 2.0 * _maskFactor,
	3.0 * _maskFactor, 4.0 * _maskFactor, 5.0 * _maskFactor, 4.0 * _maskFactor, 3.0 * _maskFactor,
	2.0 * _maskFactor, 3.0 * _maskFactor, 4.0 * _maskFactor, 3.0 * _maskFactor, 2.0 * _maskFactor,
	1.0 * _maskFactor, 2.0 * _maskFactor, 3.0 * _maskFactor, 2.0 * _maskFactor, 1.0 * _maskFactor);

float getPixelNoiseValue(int x, int y)
{
	float result = 0.0;

	vec2 texelSize = vec2(1.0 / 2048.0);
	vec2 screenPos = vec2(float(x), float(y));

	for (uint i = 0u; i < 25u; i++)
	{
		vec2 pos = clamp(screenPos + _pixelOffset[i], 0.0, 1024.0);
		vec2 iidx = pos * texelSize;
		result += perlinNoise(iidx, perlinScale, perlinFrecuency, perlinAmplitude, perlinOctaves) * _weights[i];
	}

	return result;
}

void main()
{
	ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

	float dx = 1.0 / 2048.0;
	float dy = 1.0 / 2048.0;
	vec2 uv = vec2(float(pixel.x) * dx, float(pixel.y) * dy);
	vec2 suv = uv + 5.5;
	float coverage = perlinNoise(uv, perlinScale, perlinFrecuency, perlinAmplitude, perlinOctaves);
	float cloudType = 0.5;//perlinNoise(suv, perlinScale, perlinFrecuency, perlinAmplitude, perlinOctaves);
	//float coverage = getPixelNoiseValue(pixel.x, pixel.y);


	vec4 weather = vec4(coverage, clamp(coverage, 0.0, 1.0), 0, 1);

	imageStore(outWeatherTex, pixel, weather);
}
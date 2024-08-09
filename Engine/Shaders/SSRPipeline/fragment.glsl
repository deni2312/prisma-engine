#version 460 core
#extension GL_ARB_bindless_texture : enable

in vec2 UV;
out vec4 outColor;

layout(bindless_sampler) uniform sampler2D textureAlbedo;
layout(bindless_sampler) uniform sampler2D textureNorm;
layout(bindless_sampler) uniform sampler2D texturePosition;
layout(bindless_sampler) uniform sampler2D finalImage;
layout(bindless_sampler) uniform sampler2D textureDepth;

layout(std140, binding = 1) uniform MeshData {
    mat4 view;
    mat4 projection;
};

uniform float rayStep = 0.2f;
uniform int iterationCount = 100;
uniform float distanceBias = 0.05f;
uniform bool isExponentialStepEnabled = false;
uniform bool isAdaptiveStepEnabled = true;
uniform bool isBinarySearchEnabled = true;

float random(vec2 uv) {
	return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453123);
}

vec3 generatePositionFromDepth(vec2 texturePos, float depth) {
	vec4 ndc = vec4((texturePos - 0.5) * 2, depth, 1.f);
	vec4 inversed = inverse(projection) * ndc;
	inversed /= inversed.w;
	return inversed.xyz;
}

vec2 generateProjectedPosition(vec3 pos) {
	vec4 samplePosition = projection * vec4(pos, 1.f);
	samplePosition.xy = (samplePosition.xy / samplePosition.w) * 0.5 + 0.5;
	return samplePosition.xy;
}

vec3 SSR(vec3 position, vec3 reflection) {
	vec3 step = rayStep * reflection;
	vec3 marchingPosition = position + step;
	float delta;
	float depthFromScreen;
	vec2 screenPosition;

	int i = 0;
	for (; i < iterationCount; i++) {
		screenPosition = generateProjectedPosition(marchingPosition);
		depthFromScreen = abs(generatePositionFromDepth(screenPosition, texture(textureDepth, screenPosition).x).z);
		delta = abs(marchingPosition.z) - depthFromScreen;
		if (abs(delta) < distanceBias) {
			vec3 color = vec3(1);
			return texture(finalImage, screenPosition).xyz * color;
		}
		if (isBinarySearchEnabled && delta > 0) {
			break;
		}
		if (isAdaptiveStepEnabled) {
			float directionSign = sign(abs(marchingPosition.z) - depthFromScreen);
			step = step * (1.0 - rayStep * max(directionSign, 0.0));
			marchingPosition += step * (-directionSign);
		}
		else {
			marchingPosition += step;
		}
		if (isExponentialStepEnabled) {
			step *= 1.05;
		}
	}
	if (isBinarySearchEnabled) {
		for (; i < iterationCount; i++) {

			step *= 0.5;
			marchingPosition = marchingPosition - step * sign(delta);

			screenPosition = generateProjectedPosition(marchingPosition);
			depthFromScreen = abs(generatePositionFromDepth(screenPosition, texture(textureDepth, screenPosition).x).z);
			delta = abs(marchingPosition.z) - depthFromScreen;

			if (abs(delta) < distanceBias) {
				vec3 color = vec3(1);
				return texture(finalImage, screenPosition).xyz * color;
			}
		}
	}
	return vec3(0.0);
}


void main(void)
{
    vec3 albedo = texture(finalImage, UV).rgb;
    float metallic = texture(textureAlbedo, UV).a;
	vec3 position = generatePositionFromDepth(UV, texture(textureDepth, UV).x);
	vec4 normal = view * vec4(texture(textureNorm, UV).xyz, 0.0);
	if (metallic < 0.01) {
		outColor = texture(finalImage, UV);
	}
	else {
		vec3 reflectionDirection = normalize(reflect(position, normalize(normal.xyz)));
		outColor = vec4(SSR(position, normalize(reflectionDirection)), 1.f);
		if (outColor.xyz == vec3(0.f)) {
			outColor = texture(finalImage, UV);
		}
	}
}
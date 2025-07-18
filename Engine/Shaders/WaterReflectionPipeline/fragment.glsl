#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2D screenTexture;
uniform texture2D waterMaskTexture;
uniform texture2D depthTexture;
uniform sampler screenTexture_sampler; // Sampler for screenTexture

uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos; // Camera's world position
};

/**
 * @brief Reconstructs the view-space position from screen UV coordinates and normalized device coordinate (NDC) depth.
 * @param uv Normalized screen coordinates [0,1].
 * @param depth Depth value sampled from the depth texture (assumed to be NDC depth in [0,1]).
 * @return The 3D position in view space.
 */
vec3 reconstructViewPos(vec2 uv, float depth)
{
    // Convert UV to NDC (Normalized Device Coordinates) [-1, 1]
    // The depth value is also converted from [0,1] to NDC [-1,1]
    vec4 clipSpacePos = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);

    // Transform from clip space to view space using the inverse projection matrix
    vec4 viewSpacePos = inverse(projection) * clipSpacePos;

    // Perform perspective divide to get the correct view-space coordinates
    return viewSpacePos.xyz / viewSpacePos.w;
}

/**
 * @brief Reconstructs the world-space position from screen UV coordinates and normalized device coordinate (NDC) depth.
 * @param uv Normalized screen coordinates [0,1].
 * @param depth Depth value sampled from the depth texture (assumed to be NDC depth in [0,1]).
 * @return The 3D position in world space.
 */
vec3 reconstructWorldPos(vec2 uv, float depth)
{
    // First, reconstruct the position in view space
    vec3 viewSpacePos = reconstructViewPos(uv, depth);
    // Then, transform from view space to world space using the inverse view matrix
    return (inverse(view) * vec4(viewSpacePos, 1.0)).xyz;
}

const float rayStep = 0.2f;
const int iterationCount = 100;
const float distanceBias = 0.05f;
const bool isExponentialStepEnabled = false;
const bool isAdaptiveStepEnabled = true;
const bool isBinarySearchEnabled = true;

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
		depthFromScreen = abs(generatePositionFromDepth(screenPosition, texture(sampler2D(depthTexture,screenTexture_sampler), screenPosition).x).z);
		delta = abs(marchingPosition.z) - depthFromScreen;
		if (abs(delta) < distanceBias) {
			vec3 color = vec3(1);
			return texture(sampler2D(screenTexture,screenTexture_sampler), screenPosition).xyz * color;
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
			depthFromScreen = abs(generatePositionFromDepth(screenPosition, texture(sampler2D(depthTexture,screenTexture_sampler), screenPosition).x).z);
			delta = abs(marchingPosition.z) - depthFromScreen;

			if (abs(delta) < distanceBias) {
				vec3 color = vec3(1);
				return texture(sampler2D(screenTexture,screenTexture_sampler), screenPosition).xyz * color;
			}
		}
	}
	return vec3(0.0);
}


void main(void)
{
    vec3 albedo = texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords).rgb;
    float metallic = texture(sampler2D(waterMaskTexture,screenTexture_sampler), TexCoords).r;
	vec3 position = generatePositionFromDepth(TexCoords, texture(sampler2D(depthTexture,screenTexture_sampler), TexCoords).x);
	vec4 normal = view * vec4(texture(sampler2D(waterMaskTexture,screenTexture_sampler), TexCoords).xyz, 0.0);
	if (metallic < 0.01) {
		FragColor = texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords);
	}
	else {
		vec3 reflectionDirection = normalize(reflect(position, normalize(normal.xyz)));
		FragColor = vec4(SSR(position, normalize(reflectionDirection)), 1.f);
		if (FragColor.xyz == vec3(0.f)) {
			FragColor = texture(sampler2D(screenTexture,screenTexture_sampler), TexCoords);
		}
	}
}
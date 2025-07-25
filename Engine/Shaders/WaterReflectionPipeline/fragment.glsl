#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 TexCoords;

uniform ViewProjection
{
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

uniform texture2D screenTexture;
uniform texture2D waterMaskTexture;
uniform texture2D positionTexture;
uniform sampler screenTexture_sampler;

const float rayStep = 0.2f;
const int iterationCount = 100;
const float distanceBias = 0.05f;
const bool enableSSR = false;
const int sampleCount = 4;
const bool isSamplingEnabled = false;
const bool isExponentialStepEnabled = false;
const bool isAdaptiveStepEnabled = true;
const bool isBinarySearchEnabled = true;
const bool debugDraw = false;

float random (vec2 uv) {
	return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453123); //simple random function
}

vec3 generatePositionFromDepth(vec2 texturePos, float depth) {
	return vec3(view*vec4(texture(sampler2D(positionTexture,screenTexture_sampler),texturePos).rgb,1));
}

vec2 generateProjectedPosition(vec3 pos){
	vec4 samplePosition = projection * vec4(pos, 1.f);

	samplePosition.y=-samplePosition.y;

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
		depthFromScreen = abs(generatePositionFromDepth(screenPosition, 0).z);
		delta = abs(marchingPosition.z) - depthFromScreen;
		if (abs(delta) < distanceBias) {
			vec3 color = vec3(1);
			if(debugDraw)
				color = vec3( 0.5+ sign(delta)/2,0.3,0.5- sign(delta)/2);
			return texture(sampler2D(screenTexture,screenTexture_sampler), screenPosition).xyz * color;
		}
		if (isBinarySearchEnabled && delta > 0) {
			break;
		}
		if (isAdaptiveStepEnabled){
			float directionSign = sign(abs(marchingPosition.z) - depthFromScreen);
			//this is sort of adapting step, should prevent lining reflection by doing sort of iterative converging
			//some implementation doing it by binary search, but I found this idea more cheaty and way easier to implement
			step = step * (1.0 - rayStep * max(directionSign, 0.0));
			marchingPosition += step * (-directionSign);
		}
		else {
			marchingPosition += step;
		}
		if (isExponentialStepEnabled){
			step *= 1.05;
		}
    }
	if(isBinarySearchEnabled){
		for(; i < iterationCount; i++){
			
			step *= 0.5;
			marchingPosition = marchingPosition - step * sign(delta);
			
			screenPosition = generateProjectedPosition(marchingPosition);
			depthFromScreen = abs(generatePositionFromDepth(screenPosition, 0).z);
			delta = abs(marchingPosition.z) - depthFromScreen;
			
			if (abs(delta) < distanceBias) {
                vec3 color = vec3(1);
                if(debugDraw)
                    color = vec3( 0.5+ sign(delta)/2,0.3,0.5- sign(delta)/2);
				return texture(sampler2D(screenTexture,screenTexture_sampler), screenPosition).xyz * color;
			}
		}
	}
	
    return vec3(0.0);
}

void main(){
	vec2 UV=TexCoords;

	vec3 position = generatePositionFromDepth(UV, 0);
	vec4 normal = view * vec4(texture(sampler2D(waterMaskTexture,screenTexture_sampler), UV).xyz, 0.0);
	float metallic = texture(sampler2D(waterMaskTexture,screenTexture_sampler), UV).a;
	if (metallic < 0.01) {
		FragColor = texture(sampler2D(screenTexture,screenTexture_sampler), UV);
	} else {
		vec3 reflectionDirection = normalize(reflect(position, normalize(normal.xyz)));
		FragColor = vec4(SSR(position, normalize(reflectionDirection)), 1.f);
		if (FragColor.xyz == vec3(0.f)){
			FragColor = texture(sampler2D(screenTexture,screenTexture_sampler), UV);
		}
	}
}
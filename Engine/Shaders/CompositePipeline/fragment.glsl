#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 TexCoords;

uniform texture2D accum;
uniform texture2D reveal;
uniform sampler accum_sampler;

// epsilon number
const float EPSILON = 0.00001f;

// calculate floating point numbers equality accurately
bool isApproximatelyEqual(float a, float b)
{
	return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

// get the max value between three values
float max3(vec3 v) 
{
	return max(max(v.x, v.y), v.z);
}

uniform ConstantsClusters
{
    float zNear;
    float zFar;
    vec2 padding2;
    mat4 inverseProjection;
    ivec4 gridSize;
    ivec4 screenDimensions;
};

void main()
{

    // fragment coordination
	ivec2 coords = ivec2(gl_FragCoord.xy);
	// fragment revealage
	float revealage = texelFetch(sampler2D(reveal,accum_sampler), coords, 0).r;
	
	// save the blending and color texture fetch cost if there is not a transparent fragment
	if (isApproximatelyEqual(revealage, 1.0f)) 
		discard;
 
	// fragment color
	vec4 accumulation = texelFetch(sampler2D(accum,accum_sampler), coords, 0);
	
	// suppress overflow
	if (isinf(max3(abs(accumulation.rgb)))) 
		accumulation.rgb = vec3(accumulation.a);

	// prevent floating point precision bug
	vec3 average_color = accumulation.rgb / max(accumulation.a, EPSILON);

	// blend pixels
	FragColor = vec4(average_color, 1.0f - revealage);
    //float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
}

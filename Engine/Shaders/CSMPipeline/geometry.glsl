layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

layout(set=0, binding=0, std430) uniform LightSpaceMatrices
{
	mat4 lightSpaceMatrices[16];
	float cascadePlanes[16];
    float sizeCSM;
    float farPlaneCSM;
    vec2 paddingCSM;
};

void main()
{
	for (int i = 0; i < 3; ++i)
	{
		gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}
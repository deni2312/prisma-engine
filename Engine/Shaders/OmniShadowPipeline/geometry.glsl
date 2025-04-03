layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform ShadowMatrices{
    mat4 shadowMatrices[6];
};

layout(location = 0) out vec4 FragPos;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}
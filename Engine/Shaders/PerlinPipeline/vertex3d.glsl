layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

layout(location = 0) out vec2 TexCoords;
layout(location = 1) out vec3 FragPos;


uniform Constants{
    mat4 view;
    mat4 projection;
};

void main()
{
    TexCoords = aTexCoords;
    FragPos=aPos;
    gl_Position = projection*view*vec4(aPos, 1.0);
}
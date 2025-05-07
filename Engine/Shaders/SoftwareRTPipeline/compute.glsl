#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_samplerless_texture_functions : require

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(binding = 1, rgba8) writeonly uniform image2D screenTexture;

uniform sampler textureRepeat_sampler;

uniform texture2D diffuseTexture[];

uniform ViewProjection {
    mat4 view;
    mat4 projection;
    vec4 viewPos;
};

struct SizeMeshes {
    uint vertexBase;
    uint indexBase;
    uint vertexSize;
    uint indexSize;
};

/*struct Triangle {
    vec4 v0, v1, v2;
    vec4 index;
};

struct AABB {
    vec4 min;
    vec4 max;
};

struct BVHNode {
    AABB bounds;
    vec4 leftFirst;
    vec4 count;
};

buffer verticesBVH {
    Triangle triangle_data[];
};

buffer nodesBVH {
    BVHNode node_data[];
};*/

uniform TotalSizes {
    ivec4 totalMeshes;
};

struct Vertex {
    vec4 vertex;
    vec4 uv;
};

buffer vertices {
    Vertex vertices_data[];
};

buffer indices {
    uint indices_data[];
};

buffer SizeData {
    SizeMeshes size_data[];
};

struct MeshData {
    mat4 model;
    mat4 normal;
};

readonly buffer models {
    MeshData modelsData[];
};

bool rayTriangleIntersect(
    vec3 orig, vec3 dir,
    vec3 v0, vec3 v1, vec3 v2,
    out float t, out vec3 bary)
{
    float EPSILON = 0.000001;
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 h = cross(dir, edge2);
    float a = dot(edge1, h);
    if (abs(a) < EPSILON) return false;

    float f = 1.0 / a;
    vec3 s = orig - v0;
    float u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return false;

    vec3 q = cross(s, edge1);
    float v = f * dot(dir, q);
    if (v < 0.0 || u + v > 1.0) return false;

    float tTemp = f * dot(edge2, q);
    if (tTemp > EPSILON) {
        t = tTemp;
        bary = vec3(1.0 - u - v, u, v);
        return true;
    }
    return false;
}

void main()
{
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
    ivec2 screenSize = imageSize(screenTexture);

    vec2 uv = (vec2(gid) + 0.5) / vec2(screenSize) * 2.0 - 1.0;

    vec4 rayClip = vec4(uv, -1.0, 1.0);
    vec4 rayView = inverse(projection) * rayClip;
    rayView = vec4(rayView.xy, -1.0, 0.0);

    vec3 rayOrigin = viewPos.xyz;
    vec3 rayDir = normalize((inverse(view) * rayView).xyz);

    float closestT = 1e30;
    bool hit = false;
    int found = -1;
    vec2 finalUV = vec2(0.0);

    for (int i = 0; i < totalMeshes.r; i++) {
        SizeMeshes currentSize = size_data[i];
        mat4 model = modelsData[i].model;

        for (uint tri = currentSize.indexBase; tri < currentSize.indexBase + currentSize.indexSize; tri += 3) {
            uint i0 = indices_data[tri + 0];
            uint i1 = indices_data[tri + 1];
            uint i2 = indices_data[tri + 2];

            Vertex vert0 = vertices_data[currentSize.vertexBase + i0];
            Vertex vert1 = vertices_data[currentSize.vertexBase + i1];
            Vertex vert2 = vertices_data[currentSize.vertexBase + i2];

            vec3 v0 = (model * vert0.vertex).xyz;
            vec3 v1 = (model * vert1.vertex).xyz;
            vec3 v2 = (model * vert2.vertex).xyz;

            float t;
            vec3 bary;
            if (rayTriangleIntersect(rayOrigin, rayDir, v0, v1, v2, t, bary)) {
                if (t < closestT) {
                    closestT = t;
                    hit = true;
                    found = i;
                    finalUV = vert0.uv.xy * bary.x + vert1.uv.xy * bary.y + vert2.uv.xy * bary.z;
                    break;
                }
            }
        }
        if(hit) {
            break;
        }
    }

    if (hit) {
        vec4 diffuse = texture(sampler2D(diffuseTexture[found], textureRepeat_sampler), finalUV);
        imageStore(screenTexture, gid, vec4(diffuse.rgb, 1.0));
    } else {
        imageStore(screenTexture, gid, vec4(0.0, 0.0, 0.0, 1.0));
    }
}

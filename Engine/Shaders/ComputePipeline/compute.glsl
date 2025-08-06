layout ( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;


uniform ConstantsClusters
{
float zNear;
float zFar;
vec2 padding;
mat4 inverseProjection;
ivec4 gridSize;
ivec4 screenDimensions;
};

struct Cluster
{
    vec4 minPoint;
    vec4 maxPoint;
    uint count;
    uint lightIndices[100];
};

layout(binding=0)
buffer clusters
{
    Cluster clusters_data[];
};



// Returns the intersection point of an infinite line and a
// plane perpendicular to the Z-axis
vec3 lineIntersectionWithZPlane(vec3 startPoint, vec3 endPoint, float zDistance)
{
    vec3 direction = endPoint - startPoint;
    vec3 normal = vec3(0.0, 0.0, -1.0); // plane normal

    // skip check if the line is parallel to the plane.

    float t = (zDistance - dot(normal, startPoint)) / dot(normal, direction);
    return startPoint + t * direction; // the parametric form of the line equation
}
vec3 screenToView(vec2 screenCoord)
{
    //Vulkan uses z = 0 OpenGL uses z = -1 
    vec4 ndc = vec4(screenCoord / screenDimensions.xy * 2.0 - 1.0,0, 1.0);

    vec4 viewCoord = inverseProjection * ndc;
    viewCoord = viewCoord / viewCoord.w;
    return viewCoord.xyz;
}

void main()
{

    // Eye position is zero in view space
    vec3 eyePos = vec3(0.0);

    uint tileIndex = gl_WorkGroupID.x + gl_WorkGroupID.y * gridSize.x +
        gl_WorkGroupID.z * gridSize.x * gridSize.y;
    vec2 tileSize = screenDimensions.xy / gridSize.xy;

    // calculate the min and max points of a tile in screen space
    vec2 minPoint_screenSpace = gl_WorkGroupID.xy * tileSize;
    vec2 maxPoint_screenSpace = (gl_WorkGroupID.xy + 1) * tileSize;

    // convert them to view space sitting on the near plane
    vec3 minPoint_viewSpace = screenToView(minPoint_screenSpace);
    vec3 maxPoint_viewSpace = screenToView(maxPoint_screenSpace);

    float tileNear =
        zNear * pow(zFar / zNear, gl_WorkGroupID.z / float(gridSize.z));
    float tileFar =
        zNear * pow(zFar / zNear, (gl_WorkGroupID.z + 1) / float(gridSize.z));

    // Find the 4 intersection points from the min/max points to this cluster's
    // near and far planes
    vec3 minPointNear =
        lineIntersectionWithZPlane(eyePos, minPoint_viewSpace, tileNear);
    vec3 minPointFar =
        lineIntersectionWithZPlane(eyePos, minPoint_viewSpace, tileFar);
    vec3 maxPointNear =
        lineIntersectionWithZPlane(eyePos, maxPoint_viewSpace, tileNear);
    vec3 maxPointFar =
        lineIntersectionWithZPlane(eyePos, maxPoint_viewSpace, tileFar);

    vec3 minPointAABB = min(minPointNear, minPointFar);
    vec3 maxPointAABB = max(maxPointNear, maxPointFar);

    clusters_data[tileIndex].minPoint = vec4(minPointAABB, 0.0);
    clusters_data[tileIndex].maxPoint = vec4(maxPointAABB, 0.0);

}

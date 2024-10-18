#include "../../include/Physics/DrawDebugger.h"
#include "../../include/Helpers/PrismaMath.h"
#include <glm/gtx/string_cast.hpp>

void Prisma::DrawDebugger::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
    glm::vec3 from_object=Prisma::JfromVec3(inFrom);
    glm::vec3 to_object = Prisma::JfromVec3(inTo);
    std::cout << glm::to_string(from_object) << std::endl;
    glm::vec3 color_object;
    color_object.x = inColor.r;
    color_object.y = inColor.g;
    color_object.z = inColor.b;
    line.setColor(color_object);
    line.draw(from_object, to_object);
}

void Prisma::DrawDebugger::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight)
{
}

void Prisma::DrawDebugger::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow)
{

}

JPH::DebugRenderer::Batch Prisma::DrawDebugger::CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount)
{
    return Batch();
}

JPH::DebugRenderer::Batch Prisma::DrawDebugger::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount)
{
    return Batch();
}

void Prisma::DrawDebugger::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode)
{

}

Prisma::DrawDebugger::DrawDebugger() {
    Initialize();
    m_shader = std::make_shared<Shader>("../../../Engine/Shaders/DebugPhysicsPipeline/vertex.glsl", "../../../Engine/Shaders/DebugPhysicsPipeline/fragment.glsl");
}

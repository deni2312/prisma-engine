#include "../../include/Physics/DrawDebugger.h"
#include "../../include/Helpers/PrismaMath.h"
#include <glm/gtx/string_cast.hpp>

void Prisma::DrawDebugger::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
    glm::vec3 from_object = Prisma::JfromVec3(inFrom);
    glm::vec3 to_object = Prisma::JfromVec3(inTo);
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
    /*if (m_init) {
        m_shader->use();
        m_shader->setMat4(m_modelPos, glm::mat4(1.0));
        m_shader->setVec3(m_colorPos, glm::vec3(inColor.r, inColor.g, inColor.b));
        m_vao.bind();
        std::vector<glm::vec3> v;
        v.push_back(glm::vec3(inV1.GetX(), inV1.GetY(), inV1.GetZ()));
        v.push_back(glm::vec3(inV2.GetX(), inV2.GetY(), inV2.GetZ()));
        v.push_back(glm::vec3(inV3.GetX(), inV3.GetY(), inV3.GetZ()));
        m_vbo.writeData(sizeof(glm::vec3) * v.size(), &v[0]);
        m_vao.addAttribPointer(0, 3, sizeof(Vertex), (void*)0);
        glDrawArrays(GL_TRIANGLES, 0, 3);  // Draw 'inVertexCount' vertices
        glBindVertexArray(0);
    }*/
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

void Prisma::DrawDebugger::init() {
    /*if (!m_init) {
        m_init = true;
        m_shader = std::make_shared<Shader>("../../../Engine/Shaders/DebugPhysicsPipeline/vertex.glsl", "../../../Engine/Shaders/DebugPhysicsPipeline/fragment.glsl");
        m_modelPos = m_shader->getUniformPosition("model");
        m_colorPos = m_shader->getUniformPosition("color");
    }*/
}

Prisma::DrawDebugger::DrawDebugger() {
    Initialize();
}

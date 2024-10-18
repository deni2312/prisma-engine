#pragma once
#include "Line.h"
#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include <iostream>
#include "../Containers/VAO.h"
#include "../Containers/VBO.h"
#include "../Containers/EBO.h"

namespace Prisma {

    class DrawDebugger : public JPH::DebugRenderer{
    public:
        Line line;
        virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
        virtual void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight) override;


        // Ereditato tramite DebugRenderer
        virtual void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) override;

        virtual Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override;

        virtual Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) override;

        virtual void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) override;

        void init();

        DrawDebugger();
    private:
        std::shared_ptr<Prisma::Shader> m_shader;
        unsigned int m_modelPos;
        unsigned int m_colorPos;
        Prisma::VAO m_vao;
        Prisma::VBO m_vbo;
        Prisma::EBO m_ebo;
        bool m_init = false;
    };
}
#pragma once
#ifdef JPH_DEBUG_RENDERER

#include "Line.h"
#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include <iostream>

#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Common/interface/RefCntAutoPtr.hpp";


namespace Prisma {
class DrawDebugger : public JPH::DebugRenderer {
public:
    Line line;
    void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
    void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor,
                    float inHeight) override;

    void DrawVertices(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor,
                      ECastShadow inCastShadow);
    void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor,
                      ECastShadow inCastShadow) override;

    void init();

    DrawDebugger();

protected:
    /// Implementation of DebugRenderer interface
    Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override;
    Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices,
                              int inIndexCount) override;
    void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq,
                      JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode,
                      ECastShadow inCastShadow, EDrawMode inDrawMode) override;

private:
    //std::shared_ptr<Shader> m_shader;
    unsigned int m_modelPos;
    unsigned int m_colorPos;
    //VAO m_vao;
    //VBO m_vbo;
    //EBO m_ebo;
    bool m_init = false;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_buffer;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

    struct DebugPhysicsBufferData {
        glm::vec4 color;
    };

    /// Implementation specific batch object
    class BatchImpl : public JPH::RefTargetVirtual {
    public:
        JPH_OVERRIDE_NEW_DELETE

        void AddRef() override { ++mRefCount; }
        void Release() override { if (--mRefCount == 0) delete this; }

        JPH::Array<Triangle> mTriangles;

    private:
        JPH::atomic<JPH::uint32> mRefCount = 0;
    };
};
}
#endif
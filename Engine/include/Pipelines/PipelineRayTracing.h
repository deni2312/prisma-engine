#pragma once

#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "PipelineFullScreen.h"
#include "../Helpers/Settings.h"
#include <memory>

#include "PipelineBlitRT.h"
#include "PipelinePrePass.h"

namespace Prisma {
class PipelineRayTracing {
public:
    PipelineRayTracing(const unsigned int& width = 1920, const unsigned int& height = 1080);
    void render();
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> pso();
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb();
    Diligent::RefCntAutoPtr<Diligent::ITexture> colorBuffer();
    void maxRecursion(unsigned int recursion);
    Diligent::Uint32 maxRecursion();
    void maxRecursionReflection(unsigned int recursionReflection);
    Diligent::Uint32 maxRecursionReflection();
    Diligent::Uint32 hardwareMaxReflection();
    void raytracingEasy(bool raytracingEasy);
    bool raytracingEasy();
    ~PipelineRayTracing();

private:
    unsigned int m_width;
    unsigned int m_height;

    struct RayTracingData {
        glm::vec4 CameraPos;
        glm::mat4 InvViewProj;

        glm::vec2 ClipPlanes;
        float MaxRecursion;
        float MaxRecursionReflection;
        glm::ivec4 raytracingEasy;
    };

    Diligent::Uint32 m_MaxRecursionDepth = 8;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

    Diligent::SAMPLE_COUNT m_SupportedSampleCounts;
    int m_SampleCount;
    Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pMSColorRTV;
    Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pMSDepthDSV;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

    std::function<void()> m_updateData;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_raytracingData;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_colorBuffer;

    Settings m_settings;

    std::unique_ptr<PipelineBlitRT> m_blitRT;

    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;

    Diligent::Uint32 m_maxRecursionReflection = 4;

    Diligent::Uint32 m_hardwareMaxReflection = 8;

    bool m_raytracingEasy = false;
};
}
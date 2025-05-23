#pragma once

#include "PipelineForward.h"
#include "PipelineRayTracing.h"
#include "../SceneObjects/Camera.h"
#include "PipelineDeferred.h"
#include "../Helpers/Settings.h"
#include "../../../GUI/include/ImGuiDebug.h"
#include "PipelineSkybox.h"
#include <memory>
#include "PipelineDeferredForward.h"
#include "PipelineSoftwareRT.h"

namespace Prisma {
class PipelineHandler : public InstanceData<PipelineHandler> {
public:
    struct TextureData {
        Diligent::RefCntAutoPtr<Diligent::ITexture> pColorRTV;
        Diligent::RefCntAutoPtr<Diligent::ITexture> pDepthDSV;
    };

    void initScene();
    std::shared_ptr<PipelineForward> forward();
    std::shared_ptr<PipelineDeferred> deferred();
    std::shared_ptr<PipelineDeferredForward> deferredForward();
    std::shared_ptr<PipelineRayTracing> raytracing();
    std::shared_ptr<PipelineSoftwareRT> softwareRt();
    PipelineHandler();

    Diligent::TEXTURE_FORMAT textureFormat();

    TextureData textureData();

private:
    std::shared_ptr<PipelineForward> m_forwardPipeline;
    std::shared_ptr<PipelineDeferred> m_deferredPipeline;
    std::shared_ptr<PipelineDeferredForward> m_deferredForwardPipeline;
    std::shared_ptr<PipelineRayTracing> m_raytracingPipeline;
    std::shared_ptr<PipelineSoftwareRT> m_softwarePipeline;
    SceneLoader::SceneParameters m_sceneParameters;

    Diligent::TEXTURE_FORMAT m_format;

    TextureData m_textureData;
};
}
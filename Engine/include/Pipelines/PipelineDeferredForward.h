#pragma once

#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "PipelineFullScreen.h"
#include "PipelineSSR.h"
#include <memory>

#include "PipelineDeferred.h"
#include "../Helpers/Settings.h"
#include "PipelineSSAO.h"

namespace Prisma {
class PipelineDeferredForward {
public:
    PipelineDeferredForward(const unsigned int& width, const unsigned int& height, bool srgb);
    void render();
    ~PipelineDeferredForward();

private:
    unsigned int m_width;
    unsigned int m_height;
    //std::shared_ptr<Shader> m_shader;
    //std::shared_ptr<Shader> m_shaderD;
    //std::shared_ptr<Shader> m_shaderAnimate;
    //std::shared_ptr<Shader> m_shaderCompute;
    //std::shared_ptr<Shader> m_shaderForward;
    PipelineDeferred::DeferredData m_deferredData;
    unsigned int m_positionLocation;
    unsigned int m_normalLocation;
    unsigned int m_albedoLocation;
    unsigned int m_ambientLocation;
    unsigned int m_transparentLocation;
    //std::shared_ptr<FBO> m_fbo;
    //std::shared_ptr<FBO> m_fboBuffer;
    std::shared_ptr<PipelineFullScreen> m_fullscreenPipeline;
    void showTransparencies(bool show);


    std::shared_ptr<PipelineSSR> m_ssr;
    std::shared_ptr<PipelineSSAO> m_ssao;
};
}
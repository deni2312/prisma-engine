#pragma once

#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "PipelineFullScreen.h"
#include "../Helpers/Settings.h"
#include "../Helpers/Blit.h"

#include <memory>
#include "PipelinePrePass.h"
#include "PipelineForwardTransparent.h"

namespace Prisma {
class PipelineForward {
public:
    PipelineForward(const unsigned int& width = 1920, const unsigned int& height = 1080);
    void render();
    ~PipelineForward();

private:
    unsigned int m_width;
    unsigned int m_height;

    void create();
    void createAnimation();
    void createCompositePipeline();
    void renderComposite();
    /*std::shared_ptr<Shader> m_shader;
    std::shared_ptr<Shader> m_shaderAnimate;
    std::shared_ptr<Shader> m_shaderTransparent;
    std::shared_ptr<FBO> m_fbo;
    std::shared_ptr<FBO> m_fboCopy;
    std::shared_ptr<PipelineFullScreen> m_fullscreenPipeline;

    std::shared_ptr<PipelinePrePass> m_prepass;*/

    Diligent::SAMPLE_COUNT m_SupportedSampleCounts;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoComposite;

    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbOpaque;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbComposite;


    std::function<void(Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>&, Diligent::RefCntAutoPtr<Diligent::IBuffer>&)> m_updateData;


    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoAnimation;

    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignatureAnimation;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbAnimation;

    std::function<void()> m_updateDataAnimation;

    std::unique_ptr<Prisma::PipelineForwardTransparent> m_forwardTransparent;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_compositeTexture;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_opaqueTexture;

    std::unique_ptr<Prisma::Blit> m_blit;

};
}
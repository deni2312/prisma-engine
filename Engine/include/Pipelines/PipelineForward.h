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
#include "PipelineDepthResolve.h"

namespace Prisma {
class PipelineForward {
public:
    PipelineForward(const unsigned int& width = 1920, const unsigned int& height = 1080);
    void render();
    void renderComposite();
    std::shared_ptr<Prisma::PipelineForwardTransparent> forwardTransparent();

    ~PipelineForward();

private:
    unsigned int m_width;
    unsigned int m_height;

    void create();
    void createAnimation();
    void createCompositePipeline();

    Diligent::SAMPLE_COUNT m_supportedSampleCounts;
    unsigned int m_sampleCount;
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

    std::shared_ptr<Prisma::PipelineForwardTransparent> m_forwardTransparent;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_compositeTexture;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_msaaColor;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_msaaDepth;

    void createMSAA();

    std::unique_ptr<Prisma::PipelineDepthResolve> m_depthResolve;

    Settings m_settings;

};
}